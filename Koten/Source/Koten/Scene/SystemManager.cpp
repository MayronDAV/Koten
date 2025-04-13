#include "ktnpch.h"
#include "SystemManager.h"



namespace KTN
{
	std::mutex SystemManager::m_Mutex;

	std::unordered_map<size_t, System*> SystemManager::m_Systems;


	void SystemManager::Release()
	{
		for (auto& [key, system] : m_Systems)
		{
			delete system;
		}
		m_Systems.clear();
	}


} // namespace KTN
