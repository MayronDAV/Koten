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

        auto saveState = [&](const AnimationState& p_State)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "ID" << YAML::Value << p_State.ID;
            out << YAML::Key << "Name" << YAML::Value << p_State.Name;
            out << YAML::Key << "ClipID" << YAML::Value << p_State.ClipID;
            out << YAML::Key << "Transitions" << YAML::Value << YAML::BeginSeq;
            for (auto& transition : p_State.Transitions)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "ID" << YAML::Value << transition.ID;
                out << YAML::Key << "ToState" << YAML::Value << transition.ToState;
                out << YAML::Key << "ToStateIndex" << YAML::Value << transition.ToStateIndex;
                out << YAML::Key << "HasExitTime" << YAML::Value << transition.HasExitTime;
                out << YAML::Key << "ExitTime" << YAML::Value << transition.ExitTime;
                out << YAML::Key << "BlendTime" << YAML::Value << transition.BlendTime;
                out << YAML::Key << "Conditions" << YAML::Value << YAML::BeginSeq;
                for (auto& condition : transition.Conditions)
                {
                    out << YAML::BeginMap;
                    out << YAML::Key << "ParameterID" << YAML::Value << condition.ParameterID;
                    out << YAML::Key << "Type" << YAML::Value << (uint32_t)condition.Type;
                    out << YAML::Key << "Operator" << YAML::Value << (uint32_t)condition.Operator;
                    out << YAML::Key << "CompareValue" << YAML::Value;
                    if (condition.Type == ParameterType::Bool)
                        out << condition.CompareValue.Bool;
                    else if (condition.Type == ParameterType::Float)
                        out << condition.CompareValue.Float;
                    else if (condition.Type == ParameterType::Int)
                        out << condition.CompareValue.Int;
                    out << YAML::EndMap;
                }
                out << YAML::EndSeq;
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;
        };

        out << YAML::Key << "AssetHandle" << YAML::Value << p_Controller->Handle;
        out << YAML::Key << "AnimationHandle" << YAML::Value << p_Controller->AnimationHandle;

        out << YAML::Key << "Parameters" << YAML::Value << YAML::BeginSeq;
        for (auto& param : p_Controller->Parameters)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "ID" << YAML::Value << param.ID;
            out << YAML::Key << "Name" << YAML::Value << param.Name;
            out << YAML::Key << "Type" << YAML::Value << (uint32_t)param.Type;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        out << YAML::Key << "EntryState" << YAML::Value << YAML::BeginSeq;
        saveState(p_Controller->EntryState);
        out << YAML::EndSeq;
        out << YAML::Key << "States" << YAML::Value << YAML::BeginSeq;
        for (auto& state : p_Controller->States)
        {
            saveState(state);
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

        auto saveState = [&](const AnimationState& p_State)
        {
            p_Out.write(reinterpret_cast<const char*>(&p_State.ID), sizeof(p_State.ID));
            Utils::WriteString(p_Out, p_State.Name);
            p_Out.write(reinterpret_cast<const char*>(&p_State.ClipID), sizeof(p_State.ClipID));

            auto transitionCount = p_State.Transitions.size();
            p_Out.write(reinterpret_cast<const char*>(&transitionCount), sizeof(transitionCount));
            for (auto& transition : p_State.Transitions)
            {
                p_Out.write(reinterpret_cast<const char*>(&transition.ID), sizeof(transition.ID));
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
                    if (condition.Type == ParameterType::Bool)
                        p_Out.write(reinterpret_cast<const char*>(&condition.CompareValue.Bool), sizeof(bool));
                    else if (condition.Type == ParameterType::Float)
                        p_Out.write(reinterpret_cast<const char*>(&condition.CompareValue.Float), sizeof(float));
                    else if (condition.Type == ParameterType::Int)
                        p_Out.write(reinterpret_cast<const char*>(&condition.CompareValue.Int), sizeof(int));
                }
            }
        };

        p_Out.write(reinterpret_cast<const char*>(&p_Controller->Handle), sizeof(AssetHandle));
        p_Out.write(reinterpret_cast<const char*>(&p_Controller->AnimationHandle), sizeof(AssetHandle));

        auto pSize = p_Controller->Parameters.size();
        p_Out.write(reinterpret_cast<const char*>(&pSize), sizeof(pSize));
        for (auto& param : p_Controller->Parameters)
        {
            AnimationController::Parameter paramData = {};
            p_Out.write(reinterpret_cast<const char*>(&paramData.ID), sizeof(paramData.ID));
            Utils::WriteString(p_Out, paramData.Name);
            p_Out.write(reinterpret_cast<const char*>(&paramData.Type), sizeof(paramData.Type));
        }

        saveState(p_Controller->EntryState);

        auto size = p_Controller->States.size();
        p_Out.write(reinterpret_cast<const char*>(&size), sizeof(size));

        for (auto& state : p_Controller->States)
        {
            saveState(state);
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

        auto loadState = [&](const auto& p_State)
        {
            AnimationState animState = {};
            animState.ID             = p_State["ID"].as<uint64_t>();
            animState.Name           = p_State["Name"].as<std::string>();
            animState.ClipID         = p_State["ClipID"].as<uint64_t>();
            const auto& transitions  = p_State["Transitions"];
            if (transitions)
            {
                for (auto& transition : transitions)
                {
                    AnimationTransition animTransition = {};
                    animTransition.ID                  = transition["ID"].as<uint64_t>();
                    animTransition.ToState             = transition["ToState"].as<uint64_t>();
                    animTransition.ToStateIndex        = transition["ToStateIndex"].as<uint32_t>();
                    animTransition.HasExitTime         = transition["HasExitTime"].as<bool>();
                    animTransition.ExitTime            = transition["ExitTime"].as<float>();
                    animTransition.BlendTime           = transition["BlendTime"].as<float>();
                    const auto& conditions             = transition["Conditions"];
                    if (conditions)
                    {
                        for (auto& condition : conditions)
                        {
                            AnimationCondition animCondition     = {};
                            animCondition.ParameterID            = condition["ParameterID"].as<uint64_t>();
                            animCondition.Type                   = (ParameterType)condition["Type"].as<uint32_t>();
                            animCondition.Operator               = (OperatorType)condition["Operator"].as<uint32_t>();
                            if (animCondition.Type == ParameterType::Bool)
                                animCondition.CompareValue.Bool  = condition["CompareValue"].as<bool>();
                            else if (animCondition.Type == ParameterType::Float)
                                animCondition.CompareValue.Float = condition["CompareValue"].as<float>();
                            else if (animCondition.Type == ParameterType::Int)
                                animCondition.CompareValue.Int   = condition["CompareValue"].as<int>();

                            animTransition.Conditions.push_back(animCondition);
                        }
                    }
                    animState.Transitions.push_back(animTransition);
                }
            }

            return animState;
        };

        auto controller             = CreateRef<AnimationController>();
        controller->Handle          = data["AssetHandle"].as<uint64_t>();
        controller->AnimationHandle = data["AnimationHandle"].as<uint64_t>();
        
        const auto& parameters      = data["Parameters"];
        if (parameters)
        {
            for (auto& param : parameters)
            {
                AnimationController::Parameter paramData = {};
                paramData.ID                             = param["ID"].as<uint64_t>();
                paramData.Name                           = param["Name"].as<std::string>();
                paramData.Type                           = (ParameterType)param["Type"].as<uint32_t>();
                controller->Parameters.push_back(paramData);
            }
        }

        const auto& entryState      = data["EntryState"];
        if (entryState)
            controller->EntryState  = loadState(entryState[0]);

        const auto& states          = data["States"];
        if (states)
        {
            for (auto& state : states)
            {
                auto animState      = loadState(state);
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

        auto loadState = [&]()
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
                p_In.read(reinterpret_cast<char*>(&transition.ID), sizeof(transition.ID));
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
                    if (condition.Type == ParameterType::Bool)
                        p_In.read(reinterpret_cast<char*>(&condition.CompareValue.Bool), sizeof(bool));
                    else if (condition.Type == ParameterType::Float)
                        p_In.read(reinterpret_cast<char*>(&condition.CompareValue.Float), sizeof(float));
                    else if (condition.Type == ParameterType::Int)
                        p_In.read(reinterpret_cast<char*>(&condition.CompareValue.Int), sizeof(int));

                    transition.Conditions.push_back(condition);
                }

                state.Transitions.push_back(transition);
            }

            return state;
        };

        p_In.read(reinterpret_cast<char*>(&controller->Handle), sizeof(AssetHandle));
        p_In.read(reinterpret_cast<char*>(&controller->AnimationHandle), sizeof(AssetHandle));

        size_t pSize = 0;
        p_In.read(reinterpret_cast<char*>(&pSize), sizeof(pSize));
        controller->Parameters.reserve(pSize);

        for (size_t i = 0; i < pSize; i++)
        {
            AnimationController::Parameter param = {};
            p_In.read(reinterpret_cast<char*>(&param.ID), sizeof(param.ID));
            param.Name = Utils::ReadString(p_In);
            p_In.read(reinterpret_cast<char*>(&param.Type), sizeof(param.Type));
            controller->Parameters.push_back(param);
        }

        controller->EntryState = loadState();

        size_t size = 0;
        p_In.read(reinterpret_cast<char*>(&size), sizeof(size));
        controller->States.reserve(size);

        for (size_t i = 0; i < size; i++)
        {
            auto state = loadState();
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

        auto loadState = [&]()
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
                reader.ReadBytes(&transition.ID, sizeof(transition.ID));
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
                    if (condition.Type == ParameterType::Bool)
                        reader.ReadBytes(&condition.CompareValue.Bool, sizeof(bool));
                    else if (condition.Type == ParameterType::Float)
                        reader.ReadBytes(&condition.CompareValue.Float, sizeof(float));
                    else if (condition.Type == ParameterType::Int)
                        reader.ReadBytes(&condition.CompareValue.Int, sizeof(int));

                    transition.Conditions.push_back(condition);
                }

                state.Transitions.push_back(transition);
            }

            return state;
        };

        reader.ReadBytes(&controller->Handle, sizeof(AssetHandle));
        reader.ReadBytes(&controller->AnimationHandle, sizeof(AssetHandle));

        size_t pSize = 0;
        reader.ReadBytes(&pSize, sizeof(pSize));
        controller->Parameters.reserve(pSize);

        for (size_t i = 0; i < pSize; i++)
        {
            AnimationController::Parameter param = {};
            reader.ReadBytes(&param.ID, sizeof(param.ID));
            param.Name = Utils::ReadString(reader);
            reader.ReadBytes(&param.Type, sizeof(param.Type));
            controller->Parameters.push_back(param);
        }

        controller->EntryState = loadState();

        size_t size = 0;
        reader.ReadBytes(&size, sizeof(size));
        controller->States.reserve(size);

        for (size_t i = 0; i < size; i++)
        {
            auto state = loadState();
            controller->States.push_back(state);
        }

        controller->BuildStateMap();

        return controller;
    }

    void AnimationControllerImporter::LoadBin(std::ifstream& p_In, Buffer& p_Buffer)
    {
        KTN_PROFILE_FUNCTION();

        auto loadState = [&]()
        {
            AnimationState state = {};
            p_In.read(reinterpret_cast<char*>(&state.ID), sizeof(state.ID));
            p_Buffer.Write(&state.ID, sizeof(state.ID));

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
                p_In.read(reinterpret_cast<char*>(&transition.ID), sizeof(transition.ID));
                p_Buffer.Write(&transition.ID, sizeof(transition.ID));

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

                    if (condition.Type == ParameterType::Bool)
                    {
                        bool value;
                        p_In.read(reinterpret_cast<char*>(&value), sizeof(value));
                        p_Buffer.Write(&value, sizeof(value));
                    }
                    else if (condition.Type == ParameterType::Float)
                    {
                        float value;
                        p_In.read(reinterpret_cast<char*>(&value), sizeof(value));
                        p_Buffer.Write(&value, sizeof(value));
                    }
                    else if (condition.Type == ParameterType::Int)
                    {
                        int value;
                        p_In.read(reinterpret_cast<char*>(&value), sizeof(value));
                        p_Buffer.Write(&value, sizeof(value));
                    }
                }
            }
        };

        AssetHandle handle;
        p_In.read(reinterpret_cast<char*>(&handle), sizeof(AssetHandle));
        p_Buffer.Write(&handle, sizeof(AssetHandle));

        AssetHandle animationHandle;
        p_In.read(reinterpret_cast<char*>(&animationHandle), sizeof(AssetHandle));
        p_Buffer.Write(&animationHandle, sizeof(AssetHandle));

        size_t pSize = 0;
        p_In.read(reinterpret_cast<char*>(&pSize), sizeof(pSize));
        p_Buffer.Write(&pSize, sizeof(pSize));

        for (size_t i = 0; i < pSize; i++)
        {
            uint64_t id;
            p_In.read(reinterpret_cast<char*>(&id), sizeof(id));
            p_Buffer.Write(&id, sizeof(id));

            std::string name = Utils::ReadString(p_In);
            Utils::WriteString(p_Buffer, name);

            ParameterType type;
            p_In.read(reinterpret_cast<char*>(&type), sizeof(type));
            p_Buffer.Write(&type, sizeof(type));
        }

        loadState();

        size_t size = 0;
        p_In.read(reinterpret_cast<char*>(&size), sizeof(size));
        p_Buffer.Write(&size, sizeof(size));

        for (size_t i = 0; i < size; i++)
        {
            loadState();
        }
    }

} // namespace KTN
