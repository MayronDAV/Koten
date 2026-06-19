#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Scene/Entity.h"
#include "Koten/Asset/AnimationControllerImporter.h"
#include "Koten/Asset/AnimationImporter.h"

#include <mutex>
#include <unordered_set>



namespace KTN
{
    class KTN_API AnimSystemManager
    {
        public:
            static void Init();
            static void Shutdown();

            static void RegisterScene(Scene* p_Scene);
            static void UnregisterScene(Scene* p_Scene);

            static bool IsRunning() { return m_IsRunning; }

        private:
            static void ProcessAnimations();

            static void UpdateEntity(AnimationComponent& p_AnimComp, AnimationController& p_Controller, Animation& p_Animation, float p_DeltaTime);
            static const AnimationTransition* FindTransition(AnimationComponent& p_Anim, AnimationState& p_State, AnimationController& p_Controller, Animation& p_Animation);
            static int FindFrameBinary(const std::vector<AnimationFrame>& p_Frames, double p_CurrentTime);
            static AnimationComponent::Parameter* FindParameter(AnimationComponent& p_Anim, uint64_t p_ID);
            static bool EvaluateCondition(const AnimationComponent::Parameter& p_Param, const AnimationCondition& p_Cond);
            static glm::vec4 ResolveUV(uint64_t p_RegionID, Animation& p_Animation, glm::vec2& p_OutPivot);
            static const AnimationTransition* FindEntryTransition(AnimationComponent& p_Anim, AnimationController& p_Controller);

        private:
            inline static std::unordered_set<Scene*> m_RegisteredScenes;
            inline static std::mutex m_ScenesMutex;
            inline static std::atomic<bool> m_IsRunning{ false };
            inline static bool m_TaskRegistered{ false };
    };
}