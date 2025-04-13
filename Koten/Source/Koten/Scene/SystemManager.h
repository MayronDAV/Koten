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
			static void Release();

			template <typename T, typename... Args>
			static System* RegisterSystem(Args&&... p_Args)
			{
				//std::scoped_lock<std::mutex> lock(m_Mutex);
				auto typeName = typeid(T).hash_code();
				KTN_CORE_ASSERT(!HasSystem<T>(), "Registering system more than once.");

				// Create a pointer to the system and return it so it can be used externally
				System* system = new T(std::forward<Args>(p_Args)...);
				system->OnInit();
				m_Systems[typeName] = system;
				return system;
			}

			template <typename T>
			static System* RegisterSystem(T* p_System)
			{
				//std::scoped_lock<std::mutex> lock(m_Mutex);
				auto typeName = typeid(T).hash_code();
				KTN_CORE_ASSERT(!HasSystem<T>(), "Registering system more than once.");

				// Create a pointer to the system and return it so it can be used externally
				System* system = p_System;
				system->OnInit();
				m_Systems[typeName] = system;
				return system;
			}

			template <typename T>
			static void RemoveSystem()
			{
				//std::scoped_lock<std::mutex> lock(m_Mutex);
				auto typeName = typeid(T).hash_code();
				delete m_Systems[typeName];
				m_Systems.erase(typeName);
			}

			template <typename T>
			static T* GetSystem()
			{
				//std::scoped_lock<std::mutex> lock(m_Mutex);
				auto typeName = typeid(T).hash_code();

				auto find = m_Systems.find(typeName);
				if (find != m_Systems.end())
					return dynamic_cast<T*>(find->second);

				KTN_CORE_WARN("Failed to find system");
				return nullptr;
			}

			template <typename T>
			static bool HasSystem()
			{
				//std::scoped_lock<std::mutex> lock(m_Mutex);
				auto typeName = typeid(T).hash_code();
				auto find = m_Systems.find(typeName);
				return find != m_Systems.end();
			}

			static void OnUpdate(Scene* p_Scene)
			{
				//std::scoped_lock<std::mutex> lock(m_Mutex);
				for (auto it = m_Systems.begin(); it != m_Systems.end(); ++it)
				{
					it->second->OnUpdate(p_Scene);
				}
			}

			static void OnUpdate()
			{
				//std::scoped_lock<std::mutex> lock(m_Mutex);
				for (auto it = m_Systems.begin(); it != m_Systems.end(); ++it)
				{
					it->second->OnUpdate();
				}
			}

			static void OnStart(Scene* p_Scene)
			{
				//std::scoped_lock<std::mutex> lock(m_Mutex);
				for (auto it = m_Systems.begin(); it != m_Systems.end(); ++it)
				{
					it->second->OnStart(p_Scene);
				}
			}

			static void OnStop(Scene* p_Scene)
			{
				//std::scoped_lock<std::mutex> lock(m_Mutex);
				for (auto it = m_Systems.begin(); it != m_Systems.end(); ++it)
				{
					it->second->OnStop(p_Scene);
				}
			}

		private:
			static std::mutex m_Mutex;

			static std::unordered_map<size_t, System*> m_Systems;
	};
} // namespace KTN
