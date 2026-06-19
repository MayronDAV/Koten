#include "ktnpch.h"
#include "AnimSystem.h"
#include "AnimSystemManager.h"



namespace KTN
{

    bool AnimSystem::OnStart(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

        AnimSystemManager::RegisterScene(p_Scene);

        return true;
    }

    bool AnimSystem::OnStop(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

        AnimSystemManager::UnregisterScene(p_Scene);

        auto& registry = p_Scene->GetRegistry();
        registry.view<AnimationComponent>().each([&](auto p_Entity, AnimationComponent& p_AnimComp)
        {
            auto entt  = Entity(p_Entity, p_Scene);
    
            if (!entt.IsActive() || !entt.IsEnabled())
                return;
    
            p_AnimComp.CurrentState = InvalidAnimState;
            p_AnimComp.CurrentFrame = 0;
            p_AnimComp.Direction    = 1;
            p_AnimComp.StateTime    = 0.0f;
            p_AnimComp.IsPlaying    = false;
        });

        return true;
    }

    bool AnimSystem::OnInit()
    {
        KTN_PROFILE_FUNCTION();

        return true;
    }

    void AnimSystem::OnUpdate(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

    }

} // namespace KTN
