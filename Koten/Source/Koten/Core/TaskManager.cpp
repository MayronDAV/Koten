#include "ktnpch.h"
#include "TaskManager.h"



namespace KTN
{
    TaskManager& TaskManager::Get()
    {
        KTN_PROFILE_FUNCTION();

        if (!s_Instance)
        {
            KTN_CORE_ERROR("TaskManager::Get() called but instance is null!");
            throw std::runtime_error("TaskManager not initialized");
        }

        return *s_Instance;
    }

    void TaskManager::Create()
    {
        KTN_PROFILE_FUNCTION();

        if (s_Instance)
        {
            KTN_CORE_WARN("TaskManager already initialized!");
            return;
        }

        s_Instance = new TaskManager();
    }

    void TaskManager::Destroy()
    {
        KTN_PROFILE_FUNCTION();

        s_Instance->ClearAllTasks();

        delete s_Instance;
        s_Instance = nullptr;
    }

    void TaskManager::AddTask(const ScheduledTask& p_Task)
    {
        KTN_PROFILE_FUNCTION();

        auto& phaseTasks = m_Tasks[p_Task.TaskPhase];

        auto it = std::find_if(phaseTasks.begin(), phaseTasks.end(),
        [&p_Task](const ScheduledTask& t) { return t.Name == p_Task.Name; });

        if (it != phaseTasks.end())
        {
            KTN_CORE_ERROR("Task '{0}' already exists in phase {1}!", p_Task.Name, static_cast<int>(p_Task.TaskPhase));
            return;
        }

        phaseTasks.push_back(p_Task);

        std::sort(phaseTasks.begin(), phaseTasks.end(),
        [](const ScheduledTask& a, const ScheduledTask& b)
        {
            return a.Priority < b.Priority;
        });
    }

    void TaskManager::TryAddTask(const ScheduledTask& p_Task)
    {
        KTN_PROFILE_FUNCTION();

        auto& phaseTasks = m_Tasks[p_Task.TaskPhase];

        auto it = std::find_if(phaseTasks.begin(), phaseTasks.end(),
        [&p_Task](const ScheduledTask& t) { return t.Name == p_Task.Name; });

        if (it != phaseTasks.end())
            return;

        phaseTasks.push_back(p_Task);

        std::sort(phaseTasks.begin(), phaseTasks.end(),
        [](const ScheduledTask& a, const ScheduledTask& b)
        {
            return a.Priority < b.Priority;
        });
    }

    void TaskManager::RemoveTask(const std::string& p_Name)
    {
        KTN_PROFILE_FUNCTION();

        for (auto& [phase, tasks] : m_Tasks)
        {
            tasks.erase(
                std::remove_if(tasks.begin(), tasks.end(), [&p_Name](const ScheduledTask& t) { return t.Name == p_Name; }),
                tasks.end()
            );
        }
    }

    void TaskManager::EnableTask(const std::string& p_Name)
    {
        KTN_PROFILE_FUNCTION();

        for (auto& [phase, tasks] : m_Tasks)
        {
            for (auto& task : tasks)
            {
                if (task.Name == p_Name)
                {
                    task.Enabled = true;
                    return;
                }
            }
        }
    }

    void TaskManager::DisableTask(const std::string& p_Name)
    {
        KTN_PROFILE_FUNCTION();

        for (auto& [phase, tasks] : m_Tasks)
        {
            for (auto& task : tasks)
            {
                if (task.Name == p_Name)
                {
                    task.Enabled = false;
                    return;
                }
            }
        }
    }

    void TaskManager::ExecuteInit()
    {
        KTN_PROFILE_FUNCTION();

        ExecutePhase(Phase::Init);
        WaitForAll();
    }

    void TaskManager::ExecuteDestroy()
    {
        KTN_PROFILE_FUNCTION();

        ExecutePhase(Phase::PreDestroy);
        ExecutePhase(Phase::Destroy);
        WaitForAll();
    }

    void TaskManager::ExecutePhase(Phase p_Phase)
    {
        KTN_PROFILE_FUNCTION();

        auto it = m_Tasks.find(p_Phase);
        if (it == m_Tasks.end()) return;

        auto& tasks = it->second;

        m_ParallelFutures.clear();

        std::vector<ScheduledTask*> serialTasks;
        std::vector<ScheduledTask*> parallelTasks;

        for (auto& task : tasks)
        {
            if (!task.Enabled) continue;

            if (!task.Parallelizable)
                serialTasks.push_back(&task);
            else
                parallelTasks.push_back(&task);
        }

        for (auto* task : serialTasks)
            ExecuteTaskSerial(*task);

        for (auto* task : parallelTasks)
            ExecuteTaskParallel(*task);
    }

    void TaskManager::WaitForSyncPoint(SyncPoint p_Point)
    {
        KTN_PROFILE_FUNCTION();

        auto it = m_SyncFutures.find(p_Point);
        if (it == m_SyncFutures.end()) return;

        m_CurrentSyncPoint = p_Point;

        for (auto& future : it->second)
        {
            if (future.valid())
                future.wait();
        }

        it->second.clear();
        m_CurrentSyncPoint = SyncPoint::None;
    }

    void TaskManager::WaitForAll()
    {
        KTN_PROFILE_FUNCTION();

        for (auto& [point, futures] : m_SyncFutures)
        {
            for (auto& future : futures)
            {
                if (future.valid())
                    future.wait();
            }
            futures.clear();
        }
    }

    const std::vector<TaskManager::ScheduledTask>& TaskManager::GetTasksForPhase(Phase p_Phase) const
    {
        KTN_PROFILE_FUNCTION();

        static std::vector<ScheduledTask> empty;
        auto it = m_Tasks.find(p_Phase);
        return it != m_Tasks.end() ? it->second : empty;
    }

    void TaskManager::ClearAllTasks()
    {
        m_Tasks.clear();
        m_SyncFutures.clear();
    }

    void TaskManager::ExecuteTaskSerial(const ScheduledTask& p_Task)
    {
        KTN_PROFILE_FUNCTION();

        auto start = std::chrono::high_resolution_clock::now();

        try
        {
            p_Task.Function();
        }
        catch (const std::exception& e)
        {
            KTN_CORE_ERROR("Exception in task '{0}': {1}", p_Task.Name, e.what());
        }

        auto end = std::chrono::high_resolution_clock::now();

        std::promise<void> p;
        p.set_value();
        auto sharedFuture = p.get_future().share();
        m_ParallelFutures[p_Task.Name] = sharedFuture;

        const_cast<ScheduledTask&>(p_Task).LastExecutionTime = std::chrono::duration<float, std::milli>(end - start).count();
        const_cast<ScheduledTask&>(p_Task).ExecutionCount++;
    }

    void TaskManager::ExecuteTaskParallel(ScheduledTask& p_Task)
    {
        KTN_PROFILE_FUNCTION();

        auto future = ThreadManager::Get().ScheduleJobWithFuture([this, &p_Task]()
        {
            auto start  = std::chrono::high_resolution_clock::now();

            for (const auto& dep : p_Task.DependsOn)
            {
                auto it = m_ParallelFutures.find(dep);
                if (it != m_ParallelFutures.end())
                    it->second.wait();
                else
                {
                    KTN_CORE_WARN("Task '{}' depends on unknown or serial task '{}'", p_Task.Name, dep);
                }
            }

            try
            {
                p_Task.Function();
            }
            catch (const std::exception& e)
            {
                KTN_CORE_ERROR("Exception in parallel task '{0}': {1}", p_Task.Name, e.what());
            }

            auto end                                             = std::chrono::high_resolution_clock::now();
            const_cast<ScheduledTask&>(p_Task).LastExecutionTime = std::chrono::duration<float, std::milli>(end - start).count();
            const_cast<ScheduledTask&>(p_Task).ExecutionCount++;
        });

        auto sharedFuture = future.share();
        m_ParallelFutures[p_Task.Name] = sharedFuture;

        if (p_Task.TaskSyncPoint != SyncPoint::None)
            m_SyncFutures[p_Task.TaskSyncPoint].push_back(sharedFuture);
    }

} // namespace KTN
