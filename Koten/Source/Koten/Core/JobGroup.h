#pragma once
#include "Base.h"

// std
#include <mutex>
#include <condition_variable>
#include <atomic>



namespace KTN
{
    class KTN_API JobGroup
    {
        public:
            void AddJob();
            void CompleteJob();
            void Wait();

            static Ref<JobGroup> Create() { return CreateRef<JobGroup>(); }

        private:
            std::mutex m_Mutex;
            std::condition_variable m_CV;
            std::atomic<uint32_t> m_PendingJobs = 0;
    };

} // namespace KTN