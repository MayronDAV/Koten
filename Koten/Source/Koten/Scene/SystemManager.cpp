#include "ktnpch.h"
#include "SystemManager.h"



namespace KTN
{
	SystemManager::~SystemManager()
	{
		for (auto& [key, system] : m_Systems)
		{
			delete system;
		}
		m_Systems.clear();
	}

} // namespace KTN
