#include "ktnpch.h"
#include "ThreadManager.h"
#include "JobGroup.h"



namespace KTN
{
    ThreadManager& ThreadManager::Get()
    {
        KTN_PROFILE_FUNCTION();

        if (!s_Instance)
        {
            KTN_CORE_ERROR("ThreadManager::Get() called but instance is null!");
            throw std::runtime_error("ThreadManager not initialized");
        }

        return *s_Instance;
    }

    void ThreadManager::Create(uint32_t p_NumThreads)
    {
        KTN_PROFILE_FUNCTION();

        if (s_Instance)
        {
            KTN_CORE_WARN("ThreadManager already initialized!");
            return;
        }

        if (p_NumThreads == 0)
            p_NumThreads = std::max(1u, std::thread::hardware_concurrency() - 1);

        s_Instance = new ThreadManager();

        s_Instance->m_Running = true;
        s_Instance->m_Threads.reserve(p_NumThreads);

        for (uint32_t i = 0; i < p_NumThreads; ++i)
        {
            s_Instance->m_Threads.emplace_back(&ThreadManager::WorkerLoop, s_Instance, i);
            s_Instance->SetThreadName(i, "Worker_" + std::to_string(i));
        }

        s_Instance->m_Initialized = true;
        KTN_CORE_INFO("ThreadManager created with {0} threads", p_NumThreads);
    }

    void ThreadManager::Destroy()
    {
        KTN_PROFILE_FUNCTION();

        if (!s_Instance) return;

        {
            std::lock_guard<std::mutex> lock(s_Instance->m_QueueMutex);
            s_Instance->m_Running = false;
        }

        s_Instance->m_Condition.notify_all();

        for (auto& thread : s_Instance->m_Threads)
        {
            if (thread.joinable())
                thread.join();
        }

        s_Instance->m_Threads.clear();
        s_Instance->m_Initialized = false;

        delete s_Instance;
        s_Instance = nullptr;

        KTN_CORE_INFO("ThreadManager shut down");
    }

    void ThreadManager::ScheduleJob(std::function<void()> p_Job)
    {
        KTN_PROFILE_FUNCTION();

        if (!m_Running)
        {
            KTN_CORE_WARN("Attempting to schedule job on stopped ThreadManager");
            return;
        }

        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            m_Jobs.push(std::move(p_Job));
            m_ActiveJobs.fetch_add(1, std::memory_order_relaxed);
        }
        m_Condition.notify_one();
    }

    void ThreadManager::ScheduleJob(const Ref<JobGroup>& p_Group, std::function<void()> p_Job)
    {
        KTN_PROFILE_FUNCTION();

        if (!m_Running)
        {
            KTN_CORE_WARN("Attempting to schedule job on stopped ThreadManager");
            return;
        }

        if (p_Group)
            p_Group->AddJob();

        ScheduleJob([p_Group, job = std::move(p_Job)]()
        {
            job();

            if (p_Group)
                p_Group->CompleteJob();
        });
    }

    std::future<void> ThreadManager::ScheduleJobWithFuture(const Ref<JobGroup>& p_Group, std::function<void()> p_Job)
    {
        KTN_PROFILE_FUNCTION();

        if (!m_Running)
        {
            KTN_CORE_WARN("Attempting to schedule job on stopped ThreadManager");
            return std::future<void>();
        }

        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();

        if (p_Group)
            p_Group->AddJob();

        ScheduleJob([p_Group, promise, job = std::move(p_Job)]()
        {
            try
            {
                job();

                if (p_Group)
                    p_Group->CompleteJob();

                promise->set_value();
            }
            catch (...)
            {
                promise->set_exception(std::current_exception());
            }
        });

        return future;
    }

    std::future<void> ThreadManager::ScheduleJobWithFuture(std::function<void()> p_Job)
    {
        KTN_PROFILE_FUNCTION();

        if (!m_Running)
        {
            KTN_CORE_WARN("Attempting to schedule job on stopped ThreadManager");
            return std::future<void>();
        }

        auto promise = std::make_shared<std::promise<void>>();
        auto future  = promise->get_future();

        ScheduleJob([promise, job = std::move(p_Job)]()
        {
            try
            {
                job();
                promise->set_value();
            }
            catch (...)
            {
                promise->set_exception(std::current_exception());
            }
        });

        return future;
    }

    void ThreadManager::WaitForAll()
    {
        KTN_PROFILE_FUNCTION();

        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_FinishedCondition.wait(lock, [this]()
        {
            return m_ActiveJobs.load(std::memory_order_acquire) == 0 && m_Jobs.empty();
        });
    }

    uint32_t ThreadManager::GetPendingJobs() const
    {
        KTN_PROFILE_FUNCTION();

        std::lock_guard lock(m_QueueMutex);
        return (uint32_t)m_Jobs.size();
    }

    void ThreadManager::SetThreadName(uint32_t p_Index, const std::string& p_Name)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Index >= m_Threads.size()) return;

    #ifdef KTN_WINDOWS
        auto handle = m_Threads[p_Index].native_handle();
        if (handle) 
            SetThreadDescription(handle, std::wstring(p_Name.begin(), p_Name.end()).c_str());
    #elif defined(KTN_LINUX)
        auto handle = m_Threads[p_Index].native_handle();
        if (handle)
            pthread_setname_np(handle, p_Name.c_str());
    #endif
    }

    void ThreadManager::WorkerLoop(uint32_t p_ThreadIndex)
    {
        while (true)
        {
            std::function<void()> job;

            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);

                m_Condition.wait(lock, [this]() {
                    return !m_Jobs.empty() || !m_Running;
                });

                if (!m_Running && m_Jobs.empty())
                    return;

                job = std::move(m_Jobs.front());
                m_Jobs.pop();
            }

            try
            {
                job();
            }
            catch (const std::exception& e)
            {
                KTN_CORE_ERROR("Exception in worker thread {0}: {1}", p_ThreadIndex, e.what());
            }
            catch (...)
            {
                KTN_CORE_ERROR("Unknown exception in worker thread {0}", p_ThreadIndex);
            }

            m_ActiveJobs.fetch_sub(1, std::memory_order_relaxed);
            m_FinishedCondition.notify_one();
        }
    }

} // namespace KTN