#pragma once
#include "System.h"

// std
#include <mutex>
#include <unordered_map>


namespace KTN
{
	class KTN_API SystemManager
	{
		public:
			SystemManager() = default;
			~SystemManager();

			template <typename T, typename... Args>
			System* RegisterSystem(Args&&... p_Args)
			{
				auto typeName = typeid(T).hash_code();
				KTN_CORE_ASSERT(!HasSystem<T>(), "Registering system more than once.");

				// Create a pointer to the system and return it so it can be used externally
				System* system = new T(std::forward<Args>(p_Args)...);
				system->OnInit();
				m_Systems[typeName] = system;
				return system;
			}

			template <typename T>
			System* RegisterSystem(T* p_System)
			{
				auto typeName = typeid(T).hash_code();
				KTN_CORE_ASSERT(!HasSystem<T>(), "Registering system more than once.");

				// Create a pointer to the system and return it so it can be used externally
				System* system = p_System;
				system->OnInit();
				m_Systems[typeName] = system;
				return system;
			}

			template <typename T>
			void RemoveSystem()
			{
				auto typeName = typeid(T).hash_code();
				delete m_Systems[typeName];
				m_Systems.erase(typeName);
			}

			template <typename T>
			T* GetSystem()
			{
				auto typeName = typeid(T).hash_code();

				auto find = m_Systems.find(typeName);
				if (find != m_Systems.end())
					return dynamic_cast<T*>(find->second);

				KTN_CORE_WARN("Failed to find system");
				return nullptr;
			}

			template <typename T>
			bool HasSystem()
			{
				auto typeName = typeid(T).hash_code();
				auto find = m_Systems.find(typeName);
				return find != m_Systems.end();
			}

			void OnUpdate(Scene* p_Scene)
			{
				for (auto it = m_Systems.begin(); it != m_Systems.end(); ++it)
				{
					it->second->OnUpdate(p_Scene);
				}
			}

			void OnStart(Scene* p_Scene)
			{
				for (auto it = m_Systems.begin(); it != m_Systems.end(); ++it)
				{
					it->second->OnStart(p_Scene);
				}
			}

			void OnStop(Scene* p_Scene)
			{
				for (auto it = m_Systems.begin(); it != m_Systems.end(); ++it)
				{
					it->second->OnStop(p_Scene);
				}
			}

		private:
			std::unordered_map<size_t, System*> m_Systems;
	};
} // namespace KTN
