#include "ktnpch.h"
#include "AnimSystemManager.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Asset/AssetManager.h"
#include "Koten/Asset/TextureAtlasImporter.h"
#include "Koten/Core/TaskManager.h"
#include "Koten/Core/JobGroup.h"



namespace KTN
{
#define EVAL_OP(a, b)                                      \
        switch (p_Cond.Operator)                               \
        {                                                      \
            case OperatorType::Equals:          return a == b; \
            case OperatorType::NotEquals:       return a != b; \
            case OperatorType::Greater:         return a > b;  \
            case OperatorType::Less:            return a < b;  \
            case OperatorType::GreaterOrEquals: return a >= b; \
            case OperatorType::LessOrEquals:    return a <= b; \
            default: return false;                             \
        }

    void AnimSystemManager::Init()
    {
        KTN_PROFILE_FUNCTION();

        if (m_IsRunning) return;

        m_IsRunning = true;
        m_TaskRegistered = false;

        if (!m_TaskRegistered)
        {
            TaskManager::Get().TryAddTask({
                "AnimSystemManager::UpdateAll",
                TaskManager::Phase::LateUpdate,
                0,
                []()
                {
                    if (!m_IsRunning) return;
                    ProcessAnimations();
                },
                true,
                TaskManager::SyncPoint::FramePreRender
            });

            m_TaskRegistered = true;
        }
    }

    void AnimSystemManager::Shutdown()
    {
        KTN_PROFILE_FUNCTION();

        m_IsRunning = false;

        std::lock_guard<std::mutex> lock(m_ScenesMutex);
        m_RegisteredScenes.clear();
    }

    void AnimSystemManager::RegisterScene(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Scene || !m_IsRunning) return;

        std::lock_guard<std::mutex> lock(m_ScenesMutex);
        m_RegisteredScenes.insert(p_Scene);
    }

    void AnimSystemManager::UnregisterScene(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Scene) return;

        std::lock_guard<std::mutex> lock(m_ScenesMutex);
        m_RegisteredScenes.erase(p_Scene);
    }

    void AnimSystemManager::ProcessAnimations()
    {
        KTN_PROFILE_FUNCTION();

        if (!m_IsRunning) return;

        auto dt = Time::GetDeltaTime();

        std::vector<Scene*> scenesToProcess;
        {
            std::lock_guard<std::mutex> lock(m_ScenesMutex);
            scenesToProcess.assign(m_RegisteredScenes.begin(), m_RegisteredScenes.end());
        }

        struct CachedAssets
        {
            Ref<AnimationController> Controller;
            Ref<Animation> Animation;
        };
        std::unordered_map<uint64_t, CachedAssets> assetCache;


        for (auto* scene : scenesToProcess)
        {
            if (!scene) continue;

            auto& registry = scene->GetRegistry();
            auto animGroup = JobGroup::Create();

            auto view = registry.view<AnimationComponent>();

            registry.view<AnimationComponent>().each(
            [&](auto p_Entity, auto& p_AnimComp)
            {
                auto entt = Entity(p_Entity, scene);

                if (!entt.IsActive() || !entt.IsEnabled() || !p_AnimComp.IsPlaying || p_AnimComp.Controller == 0)
                    return;

                ThreadManager::Get().ScheduleJob(animGroup, [&]()
                {
                    auto cacheIt = assetCache.find(p_AnimComp.Controller);
                    if (cacheIt == assetCache.end())
                    {
                        auto controller = AssetManager::Get()->GetAsset<AnimationController>(p_AnimComp.Controller);
                        if (!controller) return;

                        auto animation = AssetManager::Get()->GetAsset<Animation>(controller->AnimationHandle);
                        if (!animation) return;

                        cacheIt = assetCache.emplace(p_AnimComp.Controller, CachedAssets{ controller, animation }).first;
                    }

                    auto& cached = cacheIt->second;
                    UpdateEntity(p_AnimComp, *cached.Controller, *cached.Animation, dt);
                });
            });

            animGroup->Wait();
        }
    }

    void AnimSystemManager::UpdateEntity(AnimationComponent& p_AnimComp, AnimationController& p_Controller, Animation& p_Animation, float p_DeltaTime)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (p_AnimComp.CurrentState == InvalidAnimState)
        {
            const AnimationTransition* entry = FindEntryTransition(p_AnimComp, p_Controller);

            if (entry)
            {
                p_AnimComp.CurrentState = entry->ToStateIndex;
                p_AnimComp.CurrentTime = 0.0f;
                p_AnimComp.CurrentFrame = 0;
                p_AnimComp.Direction = 1;
                p_AnimComp.StateTime = 0.0f;
                p_AnimComp.IsPlaying = true;
            }
            else
            {
                static int warnCount = 0;
                if (warnCount++ < 5)
                    KTN_CORE_WARN("[Anim] -> No valid Entry transition found!");
                return;
            }
        }

        AnimationState* state                 = &p_Controller.Get(p_AnimComp.CurrentState);
        const AnimationTransition* transition = FindTransition(p_AnimComp, *state, p_Controller, p_Animation);
        if (transition)
        {
            p_AnimComp.CurrentState = transition->ToStateIndex;
            p_AnimComp.CurrentTime  = 0.0f;
            p_AnimComp.CurrentFrame = 0;
            p_AnimComp.Direction    = 1;
            p_AnimComp.StateTime    = 0.0f;
            p_AnimComp.IsPlaying    = true;

            state                   = &p_Controller.Get(p_AnimComp.CurrentState);

            AnimationClip* newClip = p_Animation.Get(state->ClipID);
            if (newClip && !newClip->Frames.empty())
            {
                const AnimationFrame& frame  = newClip->Frames[0];
                glm::vec2 pivot              = {};
                p_AnimComp.CurrentAnim.UV    = ResolveUV(frame.AtlasRegionID, p_Animation, pivot);
                std::swap(p_AnimComp.CurrentAnim.UV.y, p_AnimComp.CurrentAnim.UV.w);
                p_AnimComp.CurrentAnim.Pivot = pivot;
            }
        }

        AnimationClip* clip   = p_Animation.Get(state->ClipID);

        if (!clip || clip->Frames.empty())
            return;

        uint32_t prevFrame    = p_AnimComp.CurrentFrame;
        float totalDuration   = std::max(clip->TotalDuration, 0.0001f);
        double currentTime    = (double)p_AnimComp.CurrentTime;
        double dtDouble       = (double)p_DeltaTime * (double)clip->Speed;

        p_AnimComp.StateTime += p_DeltaTime * clip->Speed;

        switch (clip->Mode)
        {
            case AnimationMode::Loop:
            {
                currentTime = fmod(currentTime + dtDouble, totalDuration);
                break;
            }
            case AnimationMode::Once:
            {
                currentTime    += dtDouble;
                if (currentTime >= totalDuration)
                {
                    currentTime          = totalDuration;
                    p_AnimComp.IsPlaying = false;
                }
                break;
            }
            case AnimationMode::PingPong:
            {
                if (p_AnimComp.Direction >= 0)
                {
                    currentTime     += dtDouble;
                    if (currentTime >= totalDuration)
                    {
                        currentTime          = totalDuration;
                        p_AnimComp.Direction = -1;
                    }
                }
                else
                {
                    currentTime     -= dtDouble;
                    if (currentTime <= 0.0)
                    {
                        currentTime          = 0.0;
                        p_AnimComp.Direction = 1;
                    }
                }
                break;
            }
        }

        int newFrame = FindFrameBinary(clip->Frames, currentTime);
        p_AnimComp.CurrentTime = (float)currentTime;

        if (newFrame != (int)p_AnimComp.CurrentFrame)
        {
            p_AnimComp.Direction = (newFrame > (int)p_AnimComp.CurrentFrame) ? 1 : -1;
            p_AnimComp.CurrentFrame = newFrame;

            const AnimationFrame& frame = clip->Frames[newFrame];

            glm::vec2 pivot;
            p_AnimComp.CurrentAnim.UV = ResolveUV(frame.AtlasRegionID, p_Animation, pivot);
            std::swap(p_AnimComp.CurrentAnim.UV.y, p_AnimComp.CurrentAnim.UV.w);
            p_AnimComp.CurrentAnim.Pivot = pivot;
        }
    }

    const AnimationTransition* AnimSystemManager::FindTransition(AnimationComponent& p_Anim, AnimationState& p_State, AnimationController& p_Controller, Animation& p_Animation)
    {
        KTN_PROFILE_FUNCTION_LOW();

        AnimationClip* clip = p_Animation.Get(p_State.ClipID);
        if (!clip) return nullptr;

        for (auto& transition : p_State.Transitions)
        {
            if (transition.ToStateIndex == p_Anim.CurrentState)
                continue;

            if (!transition.HasExitTime && transition.Conditions.empty())
                continue;

            if (transition.HasExitTime)
            {
                float normalizedTime = p_Anim.StateTime / clip->TotalDuration;
                if (normalizedTime < transition.ExitTime)
                    continue;
            }

            if (!transition.Conditions.empty())
            {
                bool valid = false;
                for (auto& cond : transition.Conditions)
                {
                    auto* param = FindParameter(p_Anim, cond.ParameterID);
                    if (!param) continue;

                    valid = EvaluateCondition(*param, cond);
                    if (valid) break;
                }

                if (!valid) continue;
            }

            return &transition;
        }

        return nullptr;
    }

    int AnimSystemManager::FindFrameBinary(const std::vector<AnimationFrame>& p_Frames, double p_CurrentTime)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (p_Frames.empty()) return 0;

        if (p_Frames.size() <= 8)
        {
            int frame = 0;
            for (size_t i = 0; i < p_Frames.size(); i++)
            {
                if ((double)p_Frames[i].Time <= p_CurrentTime)
                    frame = (int)i;
                else
                    break;
            }
            return frame;
        }

        int left = 0;
        int right = (int)p_Frames.size() - 1;
        int result = 0;

        while (left <= right)
        {
            int mid = left + (right - left) / 2;

            if ((double)p_Frames[mid].Time <= p_CurrentTime)
            {
                result = mid;
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }

        return result;
    }

    AnimationComponent::Parameter* AnimSystemManager::FindParameter(AnimationComponent& p_Anim, uint64_t p_ID)
    {
        KTN_PROFILE_FUNCTION_LOW();

        for (auto& p : p_Anim.Parameters)
        {
            if (p.ID == p_ID)
                return &p;
        }
        return nullptr;
    }

    bool AnimSystemManager::EvaluateCondition(const AnimationComponent::Parameter& p_Param, const AnimationCondition& p_Cond)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (p_Cond.Operator == OperatorType::None)
            return false;

        switch (p_Cond.Type)
        {
        case ParameterType::Bool:
        case ParameterType::Float:
        case ParameterType::Int:
        {
            EVAL_OP(p_Param.Value.Int, p_Cond.CompareValue.Int);
        }
        }

        return false;
    }

    glm::vec4 AnimSystemManager::ResolveUV(uint64_t p_RegionID, Animation& p_Animation, glm::vec2& p_OutPivot)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (p_RegionID == 0 || p_Animation.TextureAtlas == 0)
            return { 0, 0, 1, 1 };

        auto atlas = AssetManager::Get()->GetAsset<TextureAtlas>(p_Animation.TextureAtlas);
        if (!atlas)
            return { 0, 0, 1, 1 };

        const auto* region = atlas->GetRegion(p_RegionID);
        if (!region)
            return { 0, 0, 1, 1 };

        p_OutPivot = region->Pivot;
        return region->UV;
    }

    const AnimationTransition* AnimSystemManager::FindEntryTransition(AnimationComponent& p_Anim, AnimationController& p_Controller)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto& entry = p_Controller.EntryState;

        for (auto& transition : entry.Transitions)
        {
            bool valid = true;

            for (auto& cond : transition.Conditions)
            {
                auto* param = FindParameter(p_Anim, cond.ParameterID);
                if (!param || !EvaluateCondition(*param, cond))
                {
                    valid = false;
                    break;
                }
            }

            if (valid)
                return &transition;
        }

        return nullptr;
    }

} // namespace KTN
