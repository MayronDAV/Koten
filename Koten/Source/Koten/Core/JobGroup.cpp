#include "ktnpch.h"
#include "JobGroup.h"



namespace KTN
{
    void JobGroup::AddJob()
    {
        KTN_PROFILE_FUNCTION_LOW();

        m_PendingJobs.fetch_add(1, std::memory_order_relaxed);
    }

    void JobGroup::CompleteJob()
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (m_PendingJobs.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            std::lock_guard lock(m_Mutex);
            m_CV.notify_all();
        }
    }

    void JobGroup::Wait()
    {
        KTN_PROFILE_FUNCTION_LOW();

        std::unique_lock lock(m_Mutex);

        m_CV.wait(lock, [this]()
        {
            return m_PendingJobs.load(std::memory_order_acquire) == 0;
        });
    }

} // namespace KTN