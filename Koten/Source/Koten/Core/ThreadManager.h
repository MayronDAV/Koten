#pragma once
#include "Base.h"

// std
#include <functional>
#include <future>
#include <queue>
#include <chrono>


namespace KTN
{
    class JobGroup;

    class KTN_API ThreadManager
    {
        protected:
            ThreadManager() = default;
            ~ThreadManager() = default;
            ThreadManager(const ThreadManager&) = delete;
            ThreadManager& operator=(const ThreadManager&) = delete;

        public:
            static ThreadManager& Get();

            static void Create(uint32_t p_NumThreads = 0);
            static void Destroy();

            void ScheduleJob(std::function<void()> p_Job);
            std::future<void> ScheduleJobWithFuture(std::function<void()> p_Job);

            void ScheduleJob(const Ref<JobGroup>& p_Group, std::function<void()> p_Job);
            std::future<void> ScheduleJobWithFuture(const Ref<JobGroup>& p_Group, std::function<void()> p_Job);

            void WaitForAll();

            uint32_t GetNumThreads() const { return static_cast<uint32_t>(m_Threads.size()); }
            uint32_t GetPendingJobs() const;
            bool IsRunning() const { return m_Running; }

            void SetThreadName(uint32_t p_Index, const std::string& p_Name);

        private:
            void WorkerLoop(uint32_t p_ThreadIndex);

        private:
            std::vector<std::thread> m_Threads;
            std::queue<std::function<void()>> m_Jobs;

            mutable std::mutex m_QueueMutex;
            std::condition_variable m_Condition;
            std::condition_variable m_FinishedCondition;

            std::atomic<bool> m_Running{ false };
            std::atomic<uint32_t> m_ActiveJobs{ 0 };

            bool m_Initialized{ false };

            inline static ThreadManager* s_Instance = nullptr;
    };

} // namespace KTN