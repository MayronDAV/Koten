#pragma once
#include "Koten/Core/Base.h"

#include "Components.h"
#include "Scene.h"

// lib
#include <entt/entt.hpp>



namespace KTN
{
	class KTN_API Entity
	{
		public:
			Entity() = default;
			Entity(entt::entity p_Handle, Scene* p_Scene);
			Entity(const Entity&) = default;

			template<typename T, typename ...Args>
			T& AddComponent(Args&&... p_Args)
			{
				KTN_PROFILE_FUNCTION();

				KTN_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
				T& component = m_Scene->m_Registry.emplace<T>(m_Handle, std::forward<Args>(p_Args)...);
				return component;
			}

			template<typename T, typename ...Args>
			T& AddOrReplaceComponent(Args&&... p_Args)
			{
				KTN_PROFILE_FUNCTION();

				T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_Handle, std::forward<Args>(p_Args)...);
				return component;
			}

			template <typename T>
			T* TryGetComponent()
			{
				return m_Scene->GetRegistry().try_get<T>(m_Handle);
			}

			template<typename T>
			T& GetComponent()
			{
				KTN_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
				return m_Scene->m_Registry.get<T>(m_Handle);
			}

			template<typename T>
			bool HasComponent() const
			{
				return m_Scene->m_Registry.any_of<T>(m_Handle);
			}

			template<typename T>
			void RemoveComponent()
			{
				KTN_PROFILE_FUNCTION();

				KTN_CORE_ASSERT(HasComponent<T>(), "Entity does not have this component!");
				m_Scene->m_Registry.remove<T>(m_Handle);
			}

			bool IsParent(Entity p_Entity);
			void SetParent(Entity p_Entity);
			Entity GetParent();

			std::string GetTag() const;
			Scene* GetScene() const { return m_Scene; }

			void Destroy();

			bool operator==(const Entity& p_Other) const
			{
				return (m_Handle == p_Other.m_Handle) &&
					(m_Scene == p_Other.m_Scene);
			}

			bool operator!=(const Entity& p_Other) const
			{
				return !(*this == p_Other);
			}

			operator bool() const { return m_Handle != entt::null; }
			operator entt::entity() const { return m_Handle; }
			operator uint32_t() const { return (uint32_t)m_Handle; }

		private:
			entt::entity m_Handle = entt::null;
			Scene* m_Scene = nullptr;
	};
}