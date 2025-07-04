#include "ktnpch.h"
#include "Entity.h"
#include "SystemManager.h"
#include "Koten/Physics/Box2D/B2Physics.h"



namespace KTN
{
	Entity::Entity(entt::entity p_Handle, Scene* p_Scene)
		: m_Handle(p_Handle), m_Scene(p_Scene)
	{
		KTN_PROFILE_FUNCTION();
	}

	bool Entity::IsParent(Entity p_Entity)
	{
		auto nodeHierarchyComponent = TryGetComponent<HierarchyComponent>();
		if (nodeHierarchyComponent)
		{
			auto parent = nodeHierarchyComponent->Parent;
			while (parent != entt::null)
			{
				if (parent == p_Entity.m_Handle)
				{
					return true;
				}
				else
				{
					nodeHierarchyComponent = m_Scene->GetRegistry().try_get<HierarchyComponent>(parent);
					parent = nodeHierarchyComponent ? nodeHierarchyComponent->Parent : entt::null;
				}
			}
		}

		return false;
	}

	void Entity::SetParent(Entity p_Entity)
	{
		bool acceptable = false;
		auto hierarchyComponent = TryGetComponent<HierarchyComponent>();
		if (hierarchyComponent != nullptr)
		{
			acceptable = p_Entity.m_Handle != m_Handle && (!p_Entity.IsParent(*this)) && (hierarchyComponent->Parent != m_Handle);
		}
		else
			acceptable = p_Entity.m_Handle != m_Handle;

		if (!acceptable)
		{
			KTN_CORE_ERROR("Failed to parent entity!");
			return;
		}

		if (hierarchyComponent)
			HierarchyComponent::Reparent(m_Handle, p_Entity.m_Handle, m_Scene->GetRegistry(), *hierarchyComponent);
		else
		{
			AddComponent<HierarchyComponent>(p_Entity.m_Handle);
		}
	}

	Entity Entity::GetParent()
	{
		auto hierarchyComp = TryGetComponent<HierarchyComponent>();
		if (hierarchyComp)
			return Entity(hierarchyComp->Parent, m_Scene);

		return Entity(entt::null, nullptr);
	}

	UUID Entity::GetUUID() const
	{
		return m_Scene->m_Registry.get<IDComponent>(m_Handle).ID;
	}

	std::string Entity::GetTag() const
	{
		return m_Scene->m_Registry.get<TagComponent>(m_Handle).Tag;
	}

	void Entity::Destroy()
	{
		auto rbody2DComponent = TryGetComponent<Rigidbody2DComponent>();
		if (rbody2DComponent)
		{
			if (rbody2DComponent->Body != B2BodyID{})
			{
				if (m_Scene->m_SystemManager->HasSystem<B2Physics>())
					m_Scene->m_SystemManager->GetSystem<B2Physics>()->DestroyBody(rbody2DComponent->Body);
			}
		}

		auto hierarchyComponent = TryGetComponent<HierarchyComponent>();
		if (hierarchyComponent)
		{
			entt::entity child = hierarchyComponent->First;
			while (child != entt::null)
			{
				auto centt = Entity(child, m_Scene);
				auto hierarchyComponent = centt.TryGetComponent<HierarchyComponent>();
				auto next = hierarchyComponent ? hierarchyComponent->Next : entt::null;
				centt.Destroy();
				child = next;
			}
		}

		m_Scene->GetRegistry().destroy(m_Handle);
		m_Scene = nullptr;
		m_Handle = entt::null;
	}

} // namespace KTN
