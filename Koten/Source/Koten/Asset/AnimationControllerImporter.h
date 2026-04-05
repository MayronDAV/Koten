#pragma once
#include "Koten/Core/Buffer.h"
#include "Asset.h"
#include "Koten/Utils/HashCombiner.h"
#include "Koten/Core/Definitions.h"



namespace KTN
{
    struct AnimationCondition
    {
        uint64_t ParameterID                = 0;
        AnimationConditionType Type         = AnimationConditionType::Bool;
        AnimationConditionOperator Operator = AnimationConditionOperator::None;

        union
        {
            bool Bool;
            float Float;
            int Int;
        } CompareValue;
    };

    struct AnimationTransition
    {
        uint64_t ToState = 0; // Editor only, runtime will use index
        uint32_t ToStateIndex;
        std::vector<AnimationCondition> Conditions;

        bool HasExitTime = false;
        float ExitTime   = 0.0f;
        float BlendTime  = 0.1f;

        std::vector<AnimationCondition*> GetConditions(uint64_t p_ParameterID)
        {
            KTN_PROFILE_FUNCTION_LOW();
            
            std::vector<AnimationCondition*> result;
            for (auto& cond : Conditions)
            {
                if (cond.ParameterID == p_ParameterID)
                    result.push_back(&cond);
            }

            return result;
        }

        std::vector<AnimationCondition*> GetConditions(const std::string& p_Name)
        {
            KTN_PROFILE_FUNCTION_LOW();

            uint64_t id = HashString(p_Name);
            return GetConditions(id);
        }
    };

    struct AnimationState
    {
        std::string Name = "None";
        uint64_t ID      = 0;
        uint64_t ClipID  = 0;

        std::vector<AnimationTransition> Transitions;
    };

    class KTN_API AnimationController : public Asset
    {
        public:
            AnimationController() = default;
            ~AnimationController() = default;

            AssetHandle AnimationHandle = 0;
            uint64_t EntryState         = 0;
            std::vector<AnimationState> States;
            std::unordered_map<uint64_t, uint32_t> StateMap;

            AnimationState& Get(uint32_t p_Index)
            {
                return States[p_Index];
            }

            AnimationState* Get(uint64_t p_ID)
            {
                KTN_PROFILE_FUNCTION_LOW();

                if (StateMap.empty())
                    BuildStateMap();

                auto it = StateMap.find(p_ID);
                if (it == StateMap.end())
                    return nullptr;

                return &States[it->second];
            }

            uint32_t GetCount() const
            {
                return (uint32_t)States.size();
            }

            void BuildStateMap()
            {
                KTN_PROFILE_FUNCTION_LOW();

                StateMap.clear();
                StateMap.reserve(States.size());

                for (uint32_t i = 0; i < States.size(); i++)
                {
                    States[i].ID           = HashString(States[i].Name);
                    StateMap[States[i].ID] = i;
                }

                for (auto& state : States)
                {
                    for (auto& transition : state.Transitions)
                    {
                        auto it = StateMap.find(transition.ToState);
                        KTN_CORE_ASSERT(it != StateMap.end());

                        transition.ToStateIndex = it->second;
                    }
                }
            }

            ASSET_CLASS_METHODS(AnimationController)
    };

    class KTN_API AnimationControllerImporter
    {
        public:
            static Ref<AnimationController> Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
            static Ref<AnimationController> ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data);

            static void Save(const Ref<AnimationController>& p_Controller, const std::string& p_Path);
            static void SaveBin(std::ofstream& p_Out, const Ref<AnimationController>& p_Controller);

            static Ref<AnimationController> Load(const std::string& p_Path);
            static Ref<AnimationController> LoadBin(std::ifstream& p_In);
            static Ref<AnimationController> LoadBin(const Buffer& p_In);
            static void LoadBin(std::ifstream& p_In, Buffer& p_Buffer);
    };

} // namespace KTN