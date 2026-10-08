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
        KTN_PROFILE_FUNCTION();

        auto view = p_Registry.view<HierarchyComponent>();
        for (auto entity : view)
        {
            const auto hierarchy = p_Registry.try_get<HierarchyComponent>(entity);
            if (hierarchy && hierarchy->Parent == entt::null)
            {
                // Recursively update children
                UpdatePosition(p_Registry, entity);
            }
        }
    }

    void SceneGraph::UpdatePosition(entt::registry& p_Registry, entt::entity p_Entity)
    {
        KTN_PROFILE_FUNCTION();

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

            auto uiComp = p_Registry.try_get<UIComponent>(p_Entity);
            if (uiComp)
            {
                if (hierarchyComponent->Parent != entt::null)
                {
                    auto parentUIComp = p_Registry.try_get<UIComponent>(hierarchyComponent->Parent);
                    if (parentUIComp)
                    {
                        uiComp->Anchor = glm::clamp(parentUIComp->Anchor + uiComp->Offset, glm::vec2(0.0f), glm::vec2(1.0f));
                    }
                }
            }

            entt::entity child = hierarchyComponent->First;
            while (child != entt::null && p_Registry.valid(child))
            {
                auto hierarchyComponent = p_Registry.try_get<HierarchyComponent>(child);
                auto next = hierarchyComponent ? hierarchyComponent->Next : entt::null;
                UpdatePosition(p_Registry, child);
                child = next;
            }
        }
    }

} // namespace KTN
