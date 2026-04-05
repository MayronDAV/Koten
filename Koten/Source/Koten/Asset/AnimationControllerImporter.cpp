#include "ktnpch.h"
#include "AnimationControllerImporter.h"



namespace KTN
{
    Ref<AnimationController> AnimationControllerImporter::Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::AnimationController)
        {
            KTN_CORE_ERROR("Invalid asset type for animation import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        auto anim = Load(p_Metadata.FilePath);

        return anim;
    }

    Ref<AnimationController> AnimationControllerImporter::ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::AnimationController)
        {
            KTN_CORE_ERROR("Invalid asset type for animation import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        auto anim = LoadBin(p_Data);

        return anim;
    }

    void AnimationControllerImporter::Save(const Ref<AnimationController>& p_Controller, const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "AssetHandle" << YAML::Value << p_Controller->Handle;
        out << YAML::Key << "AnimationHandle" << YAML::Value << p_Controller->AnimationHandle;
        out << YAML::Key << "EntryState" << YAML::Value << p_Controller->EntryState;
        out << YAML::Key << "States" << YAML::Value << YAML::BeginSeq;
        for (auto& state : p_Controller->States)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "ID" << YAML::Value << state.ID;
            out << YAML::Key << "Name" << YAML::Value << state.Name;
            out << YAML::Key << "ClipID" << YAML::Value << state.ClipID;
            for (auto& transition : state.Transitions)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "ToState" << YAML::Value << transition.ToState;
                out << YAML::Key << "ToStateIndex" << YAML::Value << transition.ToStateIndex;
                out << YAML::Key << "HasExitTime" << YAML::Value << transition.HasExitTime;
                out << YAML::Key << "ExitTime" << YAML::Value << transition.ExitTime;
                out << YAML::Key << "BlendTime" << YAML::Value << transition.BlendTime;
                for (auto& condition : transition.Conditions)
                {
                    out << YAML::BeginMap;
                    out << YAML::Key << "ParameterID" << YAML::Value << condition.ParameterID;
                    out << YAML::Key << "Type" << YAML::Value << (uint32_t)condition.Type;
                    out << YAML::Key << "Operator" << YAML::Value << (uint32_t)condition.Operator;
                    out << YAML::Key << "CompareValue" << YAML::Value;
                    if (condition.Type == AnimationConditionType::Bool)
                        out << condition.CompareValue.Bool;
                    else if (condition.Type == AnimationConditionType::Float)
                        out << condition.CompareValue.Float;
                    else if (condition.Type == AnimationConditionType::Int)
                        out << condition.CompareValue.Int;
                    out << YAML::EndMap;
                }
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(p_Path);
        fout << out.c_str();

        KTN_CORE_INFO("AnimationController serialized to path: {}", p_Path);
    }

    void AnimationControllerImporter::SaveBin(std::ofstream& p_Out, const Ref<AnimationController>& p_Controller)
    {
        KTN_PROFILE_FUNCTION();

        p_Out.write(reinterpret_cast<const char*>(&p_Controller->Handle), sizeof(AssetHandle));
        p_Out.write(reinterpret_cast<const char*>(&p_Controller->AnimationHandle), sizeof(AssetHandle));
        p_Out.write(reinterpret_cast<const char*>(&p_Controller->EntryState), sizeof(uint64_t));

        auto size = p_Controller->States.size();
        p_Out.write(reinterpret_cast<const char*>(&size), sizeof(size));

        for (auto& state : p_Controller->States)
        {
            p_Out.write(reinterpret_cast<const char*>(&state.ID), sizeof(state.ID));
            Utils::WriteString(p_Out, state.Name);
            p_Out.write(reinterpret_cast<const char*>(&state.ClipID), sizeof(state.ClipID));

            auto transitionCount = state.Transitions.size();
            p_Out.write(reinterpret_cast<const char*>(&transitionCount), sizeof(transitionCount));
            for (auto& transition : state.Transitions)
            {
                p_Out.write(reinterpret_cast<const char*>(&transition.ToState), sizeof(transition.ToState));
                p_Out.write(reinterpret_cast<const char*>(&transition.ToStateIndex), sizeof(transition.ToStateIndex));
                p_Out.write(reinterpret_cast<const char*>(&transition.HasExitTime), sizeof(transition.HasExitTime));
                p_Out.write(reinterpret_cast<const char*>(&transition.ExitTime), sizeof(transition.ExitTime));
                p_Out.write(reinterpret_cast<const char*>(&transition.BlendTime), sizeof(transition.BlendTime));

                auto conditionCount = transition.Conditions.size();
                p_Out.write(reinterpret_cast<const char*>(&conditionCount), sizeof(conditionCount));
                for (auto& condition : transition.Conditions)
                {
                    p_Out.write(reinterpret_cast<const char*>(&condition.ParameterID), sizeof(condition.ParameterID));
                    p_Out.write(reinterpret_cast<const char*>(&condition.Type), sizeof(condition.Type));
                    p_Out.write(reinterpret_cast<const char*>(&condition.Operator), sizeof(condition.Operator));
                    if (condition.Type == AnimationConditionType::Bool)
                        p_Out.write(reinterpret_cast<const char*>(&condition.CompareValue.Bool), sizeof(bool));
                    else if (condition.Type == AnimationConditionType::Float)
                        p_Out.write(reinterpret_cast<const char*>(&condition.CompareValue.Float), sizeof(float));
                    else if (condition.Type == AnimationConditionType::Int)
                        p_Out.write(reinterpret_cast<const char*>(&condition.CompareValue.Int), sizeof(int));
                }
            }
        }
    }

    Ref<AnimationController> AnimationControllerImporter::Load(const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        if (FileSystem::GetExtension(p_Path) != ".ktcontroller")
        {
            KTN_CORE_ERROR("Failed to load file '{}'\n     Wrong extension!", p_Path);
            return nullptr;
        }

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(p_Path);
        }
        catch (const YAML::ParserException& e)
        {
            KTN_CORE_ERROR("Failed to load .ktcontroller file '{}'\n     {}", p_Path, e.what());
            return nullptr;
        }

        if (!data["AssetHandle"] || !data["AnimationHandle"])
        {
            KTN_CORE_ERROR("Failed to load file '{}'\n     Invalid AnimationController format!", p_Path);
            return nullptr;
        }

        auto controller             = CreateRef<AnimationController>();
        controller->Handle          = data["AssetHandle"].as<uint64_t>();
        controller->AnimationHandle = data["AnimationHandle"].as<uint64_t>();
        controller->EntryState      = data["EntryState"].as<uint64_t>();

        const auto& states          = data["States"];
        if (states)
        {
            for (auto& state : states)
            {
                AnimationState animState = {};
                animState.ID             = state["ID"].as<uint64_t>();
                animState.Name           = state["Name"].as<std::string>();
                animState.ClipID         = state["ClipID"].as<uint64_t>();
                const auto& transitions  = state["Transitions"];
                if (transitions)
                {
                    for (auto& transition : transitions)
                    {
                        AnimationTransition animTransition   = {};
                        animTransition.ToState      = transition["ToState"].as<uint64_t>();
                        animTransition.ToStateIndex = transition["ToStateIndex"].as<uint32_t>();
                        animTransition.HasExitTime  = transition["HasExitTime"].as<bool>();
                        animTransition.ExitTime     = transition["ExitTime"].as<float>();
                        animTransition.BlendTime    = transition["BlendTime"].as<float>();
                        const auto& conditions      = transition["Conditions"];
                        if (conditions)
                        {
                            for (auto& condition : conditions)
                            {
                                AnimationCondition animCondition              = {};
                                animCondition.ParameterID            = condition["ParameterID"].as<uint64_t>();
                                animCondition.Type                   = (AnimationConditionType)condition["Type"].as<uint32_t>();
                                animCondition.Operator               = (AnimationConditionOperator)condition["Operator"].as<uint32_t>();
                                if (animCondition.Type == AnimationConditionType::Bool)
                                    animCondition.CompareValue.Bool  = condition["CompareValue"].as<bool>();
                                else if (animCondition.Type == AnimationConditionType::Float)
                                    animCondition.CompareValue.Float = condition["CompareValue"].as<float>();
                                else if (animCondition.Type == AnimationConditionType::Int)
                                    animCondition.CompareValue.Int   = condition["CompareValue"].as<int>();

                                animTransition.Conditions.push_back(animCondition);
                            }
                        }
                        animState.Transitions.push_back(animTransition);
                    }
                }
                controller->States.push_back(animState);
            }

            controller->BuildStateMap();
        }

        return controller;
    }

    Ref<AnimationController> AnimationControllerImporter::LoadBin(std::ifstream& p_In)
    {
        KTN_PROFILE_FUNCTION();

        auto controller = CreateRef<AnimationController>();

        p_In.read(reinterpret_cast<char*>(&controller->Handle), sizeof(AssetHandle));
        p_In.read(reinterpret_cast<char*>(&controller->AnimationHandle), sizeof(AssetHandle));
        p_In.read(reinterpret_cast<char*>(&controller->EntryState), sizeof(uint64_t));

        size_t size = 0;
        p_In.read(reinterpret_cast<char*>(&size), sizeof(size));
        controller->States.reserve(size);

        for (size_t i = 0; i < size; i++)
        {
            AnimationState state = {};
            p_In.read(reinterpret_cast<char*>(&state.ID), sizeof(state.ID));
            state.Name = Utils::ReadString(p_In);
            p_In.read(reinterpret_cast<char*>(&state.ClipID), sizeof(state.ClipID));

            size_t transitionCount = 0;
            p_In.read(reinterpret_cast<char*>(&transitionCount), sizeof(transitionCount));
            state.Transitions.reserve(transitionCount);
            for (size_t j = 0; j < transitionCount; j++)
            {
                AnimationTransition transition = {};
                p_In.read(reinterpret_cast<char*>(&transition.ToState), sizeof(transition.ToState));
                p_In.read(reinterpret_cast<char*>(&transition.ToStateIndex), sizeof(transition.ToStateIndex));
                p_In.read(reinterpret_cast<char*>(&transition.HasExitTime), sizeof(transition.HasExitTime));
                p_In.read(reinterpret_cast<char*>(&transition.ExitTime), sizeof(transition.ExitTime));
                p_In.read(reinterpret_cast<char*>(&transition.BlendTime), sizeof(transition.BlendTime));

                size_t conditionCount = 0;
                p_In.read(reinterpret_cast<char*>(&conditionCount), sizeof(conditionCount));
                transition.Conditions.reserve(conditionCount);
                for (size_t k = 0; k < conditionCount; k++)
                {
                    AnimationCondition condition = {};
                    p_In.read(reinterpret_cast<char*>(&condition.ParameterID), sizeof(condition.ParameterID));
                    p_In.read(reinterpret_cast<char*>(&condition.Type), sizeof(condition.Type));
                    p_In.read(reinterpret_cast<char*>(&condition.Operator), sizeof(condition.Operator));
                    if (condition.Type == AnimationConditionType::Bool)
                        p_In.read(reinterpret_cast<char*>(&condition.CompareValue.Bool), sizeof(bool));
                    else if (condition.Type == AnimationConditionType::Float)
                        p_In.read(reinterpret_cast<char*>(&condition.CompareValue.Float), sizeof(float));
                    else if (condition.Type == AnimationConditionType::Int)
                        p_In.read(reinterpret_cast<char*>(&condition.CompareValue.Int), sizeof(int));

                    transition.Conditions.push_back(condition);
                }

                state.Transitions.push_back(transition);
            }

            controller->States.push_back(state);
        }

        controller->BuildStateMap();

        return controller;
    }

    Ref<AnimationController> AnimationControllerImporter::LoadBin(const Buffer& p_In)
    {
        KTN_PROFILE_FUNCTION();

        auto controller = CreateRef<AnimationController>();
        BufferReader reader(p_In);

        reader.ReadBytes(&controller->Handle, sizeof(AssetHandle));
        reader.ReadBytes(&controller->AnimationHandle, sizeof(AssetHandle));
        reader.ReadBytes(&controller->EntryState, sizeof(uint64_t));

        size_t size = 0;
        reader.ReadBytes(&size, sizeof(size));
        controller->States.reserve(size);

        for (size_t i = 0; i < size; i++)
        {
            AnimationState state = {};
            reader.ReadBytes(&state.ID, sizeof(state.ID));
            state.Name = Utils::ReadString(reader);
            reader.ReadBytes(&state.ClipID, sizeof(state.ClipID));

            size_t transitionCount = 0;
            reader.ReadBytes(&transitionCount, sizeof(transitionCount));
            state.Transitions.reserve(transitionCount);
            for (size_t j = 0; j < transitionCount; j++)
            {
                AnimationTransition transition = {};
                reader.ReadBytes(&transition.ToState, sizeof(transition.ToState));
                reader.ReadBytes(&transition.ToStateIndex, sizeof(transition.ToStateIndex));
                reader.ReadBytes(&transition.HasExitTime, sizeof(transition.HasExitTime));
                reader.ReadBytes(&transition.ExitTime, sizeof(transition.ExitTime));
                reader.ReadBytes(&transition.BlendTime, sizeof(transition.BlendTime));

                size_t conditionCount = 0;
                reader.ReadBytes(&conditionCount, sizeof(conditionCount));
                transition.Conditions.reserve(conditionCount);
                for (size_t k = 0; k < conditionCount; k++)
                {
                    AnimationCondition condition = {};
                    reader.ReadBytes(&condition.ParameterID, sizeof(condition.ParameterID));
                    reader.ReadBytes(&condition.Type, sizeof(condition.Type));
                    reader.ReadBytes(&condition.Operator, sizeof(condition.Operator));
                    if (condition.Type == AnimationConditionType::Bool)
                        reader.ReadBytes(&condition.CompareValue.Bool, sizeof(bool));
                    else if (condition.Type == AnimationConditionType::Float)
                        reader.ReadBytes(&condition.CompareValue.Float, sizeof(float));
                    else if (condition.Type == AnimationConditionType::Int)
                        reader.ReadBytes(&condition.CompareValue.Int, sizeof(int));

                    transition.Conditions.push_back(condition);
                }

                state.Transitions.push_back(transition);
            }
            controller->States.push_back(state);
        }

        controller->BuildStateMap();

        return controller;
    }

    void AnimationControllerImporter::LoadBin(std::ifstream& p_In, Buffer& p_Buffer)
    {
        KTN_PROFILE_FUNCTION();

        AssetHandle handle;
        p_In.read(reinterpret_cast<char*>(&handle), sizeof(AssetHandle));
        p_Buffer.Write(&handle, sizeof(AssetHandle));

        AssetHandle animationHandle;
        p_In.read(reinterpret_cast<char*>(&animationHandle), sizeof(AssetHandle));
        p_Buffer.Write(&animationHandle, sizeof(AssetHandle));

        uint64_t entryState;
        p_In.read(reinterpret_cast<char*>(&entryState), sizeof(uint64_t));
        p_Buffer.Write(&entryState, sizeof(uint64_t));

        size_t size = 0;
        p_In.read(reinterpret_cast<char*>(&size), sizeof(size));
        p_Buffer.Write(&size, sizeof(size));

        for (size_t i = 0; i < size; i++)
        {
            AnimationState state = {};
            p_In.read(reinterpret_cast<char*>(&state.ID), sizeof(state.ID));
            p_Buffer.Write(&state.ID , sizeof(state.ID));

            state.Name = Utils::ReadString(p_In);
            Utils::WriteString(p_Buffer, state.Name);

            p_In.read(reinterpret_cast<char*>(&state.ClipID), sizeof(state.ClipID));
            p_Buffer.Write(&state.ClipID, sizeof(state.ClipID));

            size_t transitionCount = 0;
            p_In.read(reinterpret_cast<char*>(&transitionCount), sizeof(transitionCount));
            p_Buffer.Write(&transitionCount, sizeof(transitionCount));
            for (size_t j = 0; j < transitionCount; j++)
            {
                AnimationTransition transition = {};
                p_In.read(reinterpret_cast<char*>(&transition.ToState), sizeof(transition.ToState));
                p_Buffer.Write(&transition.ToState, sizeof(transition.ToState));

                p_In.read(reinterpret_cast<char*>(&transition.ToStateIndex), sizeof(transition.ToStateIndex));
                p_Buffer.Write(&transition.ToStateIndex, sizeof(transition.ToStateIndex));

                p_In.read(reinterpret_cast<char*>(&transition.HasExitTime), sizeof(transition.HasExitTime));
                p_Buffer.Write(&transition.HasExitTime, sizeof(transition.HasExitTime));

                p_In.read(reinterpret_cast<char*>(&transition.ExitTime), sizeof(transition.ExitTime));
                p_Buffer.Write(&transition.ExitTime, sizeof(transition.ExitTime));

                p_In.read(reinterpret_cast<char*>(&transition.BlendTime), sizeof(transition.BlendTime));
                p_Buffer.Write(&transition.BlendTime, sizeof(transition.BlendTime));

                size_t conditionCount = 0;
                p_In.read(reinterpret_cast<char*>(&conditionCount), sizeof(conditionCount));
                p_Buffer.Write(&conditionCount, sizeof(conditionCount));
                for (size_t k = 0; k < conditionCount; k++)
                {
                    AnimationCondition condition = {};
                    p_In.read(reinterpret_cast<char*>(&condition.ParameterID), sizeof(condition.ParameterID));
                    p_Buffer.Write(&condition.ParameterID, sizeof(condition.ParameterID));

                    p_In.read(reinterpret_cast<char*>(&condition.Type), sizeof(condition.Type));
                    p_Buffer.Write(&condition.Type, sizeof(condition.Type));

                    p_In.read(reinterpret_cast<char*>(&condition.Operator), sizeof(condition.Operator));
                    p_Buffer.Write(&condition.Operator, sizeof(condition.Operator));

                    if (condition.Type == AnimationConditionType::Bool)
                    {
                        bool value;
                        p_In.read(reinterpret_cast<char*>(&value), sizeof(value));
                        p_Buffer.Write(&value, sizeof(value));
                    }
                    else if (condition.Type == AnimationConditionType::Float)
                    {
                        float value;
                        p_In.read(reinterpret_cast<char*>(&value), sizeof(value));
                        p_Buffer.Write(&value, sizeof(value));
                    }
                    else if (condition.Type == AnimationConditionType::Int)
                    {
                        int value;
                        p_In.read(reinterpret_cast<char*>(&value), sizeof(value));
                        p_Buffer.Write(&value, sizeof(value));
                    }
                }
            }
        }
    }

} // namespace KTN
