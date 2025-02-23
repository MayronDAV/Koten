#include "ktnpch.h"
#include "SceneGraph.h"



namespace KTN
{
	SceneGraph::SceneGraph()
	{
	}

	void SceneGraph::Init(entt::registry& p_Registry)
	{
		p_Registry.on_construct<HierarchyComponent>().connect<&HierarchyComponent::OnConstruct>();
		p_Registry.on_update<HierarchyComponent>().connect<&HierarchyComponent::OnUpdate>();
		p_Registry.on_destroy<HierarchyComponent>().connect<&HierarchyComponent::OnDestroy>();
	}

	void SceneGraph::DisableOnConstruct(entt::registry& p_Registry, bool p_Disable)
	{
		if (p_Disable)
			p_Registry.on_construct<HierarchyComponent>().disconnect<&HierarchyComponent::OnConstruct>();
		else
			p_Registry.on_construct<HierarchyComponent>().connect<&HierarchyComponent::OnConstruct>();
	}

	void SceneGraph::Update(entt::registry& p_Registry)
	{
		auto view = p_Registry.view<HierarchyComponent>();
		for (auto entity : view)
		{
			const auto hierarchy = p_Registry.try_get<HierarchyComponent>(entity);
			if (hierarchy && hierarchy->Parent == entt::null)
			{
				// Recursively update children
				UpdateTransform(p_Registry, entity);
			}
		}
	}

	void SceneGraph::UpdateTransform(entt::registry& p_Registry, entt::entity p_Entity)
	{
		auto hierarchyComponent = p_Registry.try_get<HierarchyComponent>(p_Entity);
		if (hierarchyComponent)
		{
			auto transform = p_Registry.try_get<TransformComponent>(p_Entity);
			if (transform)
			{
				if (hierarchyComponent->Parent != entt::null)
				{
					auto parentTransform = p_Registry.try_get<TransformComponent>(hierarchyComponent->Parent);
					if (parentTransform)
					{
						transform->SetWorldMatrix(parentTransform->GetWorldMatrix());
					}
				}
			}

			entt::entity child = hierarchyComponent->First;
			while (child != entt::null && p_Registry.valid(child))
			{
				auto hierarchyComponent = p_Registry.try_get<HierarchyComponent>(child);
				auto next = hierarchyComponent ? hierarchyComponent->Next : entt::null;
				UpdateTransform(p_Registry, child);
				child = next;
			}
		}
	}
} // namespace KTN
