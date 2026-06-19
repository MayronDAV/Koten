#pragma once
#include "Base.h"
#include "ThreadManager.h"



namespace KTN
{
    class KTN_API TaskManager
    {
        protected:
            TaskManager() = default;
            ~TaskManager() = default;
            TaskManager(const TaskManager&) = delete;
            TaskManager& operator=(const TaskManager&) = delete;

        public:
            enum class Phase
            {
                Init,
                PreUpdate,
                Update,
                LateUpdate,
                PreRender,
                Render,
                PostRender,
                PreDestroy,
                Destroy
            };

            enum class SyncPoint
            {
                None,            // Fire-and-forget
                FramePostUpdate, // after update, before render
                FramePreRender,  // before any render tasks, after update
                FrameRender,     // before render (end)
                FrameEnd,        // before next update
            };

            struct ScheduledTask
            {
                std::string Name;
                Phase TaskPhase;
                int Priority = 0;
                std::function<void()> Function;
                bool Parallelizable     = true;
                SyncPoint TaskSyncPoint = SyncPoint::FrameEnd;
                bool Enabled            = true;

                std::vector<std::string> DependsOn; // Task names that must complete before this task runs

                float LastExecutionTime = 0.0f;
                uint32_t ExecutionCount = 0;
            };

            static TaskManager& Get();

            static void Create();
            static void Destroy();

            void AddTask(const ScheduledTask& p_Task);
            void TryAddTask(const ScheduledTask& p_Task);
            void RemoveTask(const std::string& p_Name);
            void EnableTask(const std::string& p_Name);
            void DisableTask(const std::string& p_Name);

            void ExecuteInit();
            void ExecuteDestroy();
            void ExecutePhase(Phase p_Phase);

            void WaitForSyncPoint(SyncPoint p_Point);
            void WaitForAll();

            const std::vector<ScheduledTask>& GetTasksForPhase(Phase p_Phase) const;
            void ClearAllTasks();

            SyncPoint CurrentSyncPoint() const { return m_CurrentSyncPoint; }

        private:
            void ExecuteTaskSerial(const ScheduledTask& p_Task);
            void ExecuteTaskParallel(ScheduledTask& p_Task);

        private:
            std::unordered_map<Phase, std::vector<ScheduledTask>> m_Tasks;
            std::unordered_map<SyncPoint, std::vector<std::shared_future<void>>> m_SyncFutures;

            std::unordered_map<std::string, std::shared_future<void>> m_ParallelFutures;
            std::atomic<SyncPoint> m_CurrentSyncPoint = SyncPoint::None;

            inline static TaskManager* s_Instance = nullptr;
    };

} // namespace KTN