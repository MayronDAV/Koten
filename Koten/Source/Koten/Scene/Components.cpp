#include "ktnpch.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"
#include "Koten/Systems/B2Physics.h"
#include "Koten/Systems/AnimSystemManager.h"



namespace KTN
{
    void HierarchyComponent::OnConstruct(entt::registry& p_Registry, entt::entity p_Entity)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto& hierarchy = p_Registry.get<HierarchyComponent>(p_Entity);
        if (hierarchy.Parent != entt::null)
        {
            auto& parent_hierarchy = p_Registry.get_or_emplace<HierarchyComponent>(hierarchy.Parent);

            if (parent_hierarchy.First == entt::null)
            {
                parent_hierarchy.First = p_Entity;
            }
            else
            {
                // get last children
                auto prev_ent = parent_hierarchy.First;
                auto current_hierarchy = p_Registry.try_get<HierarchyComponent>(prev_ent);
                while (current_hierarchy != nullptr && current_hierarchy->Next != entt::null)
                {
                    prev_ent = current_hierarchy->Next;
                    current_hierarchy = p_Registry.try_get<HierarchyComponent>(prev_ent);
                }
                // add new
                current_hierarchy->Next = p_Entity;
                hierarchy.Prev = prev_ent;
            }

            parent_hierarchy.ChildCount++;
        }
    }

    void HierarchyComponent::OnDestroy(entt::registry& p_Registry, entt::entity p_Entity)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto& hierarchy = p_Registry.get<HierarchyComponent>(p_Entity);
        // if is the first child
        if (hierarchy.Prev == entt::null || !p_Registry.valid(hierarchy.Prev))
        {
            if (hierarchy.Parent != entt::null && p_Registry.valid(hierarchy.Parent))
            {
                auto parent_hierarchy = p_Registry.try_get<HierarchyComponent>(hierarchy.Parent);
                if (parent_hierarchy != nullptr)
                {
                    parent_hierarchy->First = hierarchy.Next;
                    if (hierarchy.Next != entt::null)
                    {
                        auto next_hierarchy = p_Registry.try_get<HierarchyComponent>(hierarchy.Next);
                        if (next_hierarchy != nullptr)
                        {
                            next_hierarchy->Prev = entt::null;
                        }
                    }

                    parent_hierarchy->ChildCount--;
                }
            }
        }
        else
        {
            auto prev_hierarchy = p_Registry.try_get<HierarchyComponent>(hierarchy.Prev);
            if (prev_hierarchy != nullptr)
            {
                prev_hierarchy->Next = hierarchy.Next;
                prev_hierarchy->ChildCount--;
            }
            if (hierarchy.Next != entt::null)
            {
                auto next_hierarchy = p_Registry.try_get<HierarchyComponent>(hierarchy.Next);
                if (next_hierarchy != nullptr)
                {
                    next_hierarchy->Prev = hierarchy.Prev;
                }
            }
        }
    }

    void HierarchyComponent::OnUpdate(entt::registry& p_Registry, entt::entity p_Entity)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto& hierarchy = p_Registry.get<HierarchyComponent>(p_Entity);
        // if is the first child
        if (hierarchy.Prev == entt::null)
        {
            if (hierarchy.Parent != entt::null)
            {
                auto parent_hierarchy = p_Registry.try_get<HierarchyComponent>(hierarchy.Parent);
                if (parent_hierarchy != nullptr)
                {
                    parent_hierarchy->First = hierarchy.Next;
                    if (hierarchy.Next != entt::null)
                    {
                        auto next_hierarchy = p_Registry.try_get<HierarchyComponent>(hierarchy.Next);
                        if (next_hierarchy != nullptr)
                        {
                            next_hierarchy->Prev = entt::null;
                        }
                    }
                }
            }
        }
        else
        {
            auto prev_hierarchy = p_Registry.try_get<HierarchyComponent>(hierarchy.Prev);
            if (prev_hierarchy != nullptr)
            {
                prev_hierarchy->Next = hierarchy.Next;
            }
            if (hierarchy.Next != entt::null)
            {
                auto next_hierarchy = p_Registry.try_get<HierarchyComponent>(hierarchy.Next);
                if (next_hierarchy != nullptr)
                {
                    next_hierarchy->Prev = hierarchy.Prev;
                }
            }
        }
    }

    void HierarchyComponent::Reparent(entt::entity p_Entity, entt::entity p_Parent, entt::registry& p_Registry, HierarchyComponent& p_Hierarchy)
    {
        KTN_PROFILE_FUNCTION_LOW();

        HierarchyComponent::OnDestroy(p_Registry, p_Entity);

        p_Hierarchy.Parent = entt::null;
        p_Hierarchy.Next = entt::null;
        p_Hierarchy.Prev = entt::null;

        if (p_Parent != entt::null)
        {
            p_Hierarchy.Parent = p_Parent;
            HierarchyComponent::OnConstruct(p_Registry, p_Entity);
        }
    }

    void PhysicsBody2D::OnComponentAdded(Scene* p_Scene, Entity* p_Entity)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!p_Scene || !p_Entity)
            return;

        if (p_Scene->GetSystemManager()->HasSystem<B2Physics>())
            p_Scene->GetSystemManager()->GetSystem<B2Physics>()->AddPhysicEntity(*p_Entity);
    }

    void PhysicsBody2D::OnComponentRemoved(Scene* p_Scene, Entity* p_Entity)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!p_Scene || !p_Entity)
            return;

        if (p_Scene->GetSystemManager()->HasSystem<B2Physics>())
            p_Scene->GetSystemManager()->GetSystem<B2Physics>()->RemovePhysicEntity(*p_Entity);
    }

    int AnimationComponent::GetParameter(const std::string& p_Name) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        for (int i = 0; i < (int)Parameters.size(); i++)
        {
            if (Parameters[i].Name == p_Name)
                return i;
        }

        return -1;
    }

    bool AnimationComponent::SetBool(const std::string& p_Name, bool p_Value)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto index = GetParameter(p_Name);
        if (index == -1) return false;

        if (Parameters[index].Type != ParameterType::Bool)
        {
            KTN_CORE_WARN("Parameter '{}' is not of type Bool", p_Name);
            return false;
        }

        Parameters[index].Value.Bool = p_Value;
        return true;
    }

    bool AnimationComponent::SetFloat(const std::string& p_Name, float p_Value)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto index = GetParameter(p_Name);
        if (index == -1) return false;

        if (Parameters[index].Type != ParameterType::Float)
        {
            KTN_CORE_WARN("Parameter '{}' is not of type Float", p_Name);
            return false;
        }

        Parameters[index].Value.Float = p_Value;
        return true;
    }

    bool AnimationComponent::SetInt(const std::string& p_Name, int p_Value)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto index = GetParameter(p_Name);
        if (index == -1) return false;

        if (Parameters[index].Type != ParameterType::Int)
        {
            KTN_CORE_WARN("Parameter '{}' is not of type Int", p_Name);
            return false;
        }

        Parameters[index].Value.Int = p_Value;
        return true;
    }

} // namespace KTN
