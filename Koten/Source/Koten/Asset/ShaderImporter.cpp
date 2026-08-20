#include "ktnpch.h"
#include "ShaderImporter.h"
#include "Koten/Asset/AssetManager.h"
#include "Koten/Asset/ShaderStage.h"

// lib
#include <yaml-cpp/yaml.h>



namespace KTN
{
    Ref<Shader> ShaderImporter::Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::Shader)
        {
            KTN_CORE_ERROR("Invalid asset type for shader import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        auto shader    = Load(p_Metadata.FilePath);
        shader->Handle = p_Handle;
        return shader;
    }

    Ref<Shader> ShaderImporter::ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::Shader)
        {
            KTN_CORE_ERROR("Invalid asset type for shader import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        BufferReader reader(p_Data);

        AssetHandle shaderHandle = 0;
        reader.ReadBytes(&shaderHandle, sizeof(shaderHandle));

        size_t size = 0;
        reader.ReadBytes(&size, sizeof(size));
        std::vector<AssetHandle> stages(size);

        for (size_t i = 0; i < size; i++)
        {
            AssetHandle handle = 0;
            reader.ReadBytes(&handle, sizeof(handle));
            stages.push_back(handle);
        }

        auto shader = Shader::Create(stages);
        shader->Handle = shaderHandle;
        return shader;
    }

    Ref<Shader> ShaderImporter::Load(const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        if (FileSystem::GetExtension(p_Path) != ".ktshader")
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
            KTN_CORE_ERROR("Failed to load .ktshader file '{}'\n     {}", p_Path, e.what());
            return nullptr;
        }

        if (!data["Handle"] || !data["Stages"])
        {
            KTN_CORE_ERROR("Failed to load file '{}'\n     Invalid shader format!", p_Path);
            return nullptr;
        }

        AssetHandle shaderHandle            = data["Handle"].as<uint64_t>();

        std::vector<AssetHandle> stages;
        for (const auto& stage : data["Stages"])
        {
            AssetHandle handle              = stage["Handle"].as<uint64_t>();
            auto type                       = (ShaderType)stage["Stage"].as<int>();
            auto path                       = stage["Path"].as<std::string>();
            auto scope                      = stage["Scope"].as<std::string>();

            if (!AssetManager::Get()->IsAssetHandleValid(handle))
            {
                AssetMetadata metadata      = {};
                metadata.Type               = AssetType::ShaderStage;
                metadata.Scope              = GetAssetScopeFromName(scope.c_str());
                metadata.FilePath           = path;
                metadata.SerializeAssetData = false;
                AssetManager::Get()->ImportAsset(handle, metadata);
            }

            stages.push_back(handle);
        }

        auto shader    = Shader::Create(stages);
        shader->Handle = shaderHandle;
        return shader;
    }

    void ShaderImporter::LoadBin(std::ifstream& p_In, Buffer& p_Buffer)
    {
        KTN_PROFILE_FUNCTION();

        AssetHandle shaderHandle = 0;
        p_In.read(reinterpret_cast<char*>(&shaderHandle), sizeof(shaderHandle));
        p_Buffer.Write(&shaderHandle, sizeof(shaderHandle));

        size_t size = 0;
        p_In.read(reinterpret_cast<char*>(&size), sizeof(size));
        p_Buffer.Write(&size, sizeof(size));
        for (size_t i = 0; i < size; i++)
        {
            AssetHandle handle = 0;
            p_In.read(reinterpret_cast<char*>(&handle), sizeof(handle));
            p_Buffer.Write(&handle, sizeof(handle));
        }
    }

    void ShaderImporter::Save(const Ref<Shader>& p_Shader, const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "Handle" << YAML::Value << p_Shader->Handle;

        out << YAML::Key << "Stages" << YAML::Value;
        out << YAML::BeginSeq;
        for (const auto& handle : p_Shader->GetStages())
        {
            auto stage = AssetManager::Get()->GetAsset<ShaderStage>(handle);
            auto metadata = AssetManager::Get()->GetMetadata(handle);
            out << YAML::BeginMap;
            out << YAML::Key << "Handle" << YAML::Value << handle;
            out << YAML::Key << "Stage" << YAML::Value << (int)stage->GetStage();
            out << YAML::Key << "Path" << YAML::Value << stage->GetPath();
            out << YAML::Key << "Scope" << YAML::Value << GetAssetScopeName(metadata.Scope);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        auto path = FileSystem::ReplaceExtension(p_Path, ".ktshader");
        std::ofstream fout(path);
        fout << out.c_str();
    }

    void ShaderImporter::Save(std::ofstream& p_Out, const Ref<Shader>& p_Shader)
    {
        KTN_PROFILE_FUNCTION();

        p_Out.write(reinterpret_cast<const char*>(&p_Shader->Handle), sizeof(p_Shader->Handle));

        const auto& stages = p_Shader->GetStages();
        size_t size        = stages.size();
        p_Out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        for (const auto& handle : stages)
        {
            auto stage     = AssetManager::Get()->GetAsset<ShaderStage>(handle);
            p_Out.write(reinterpret_cast<const char*>(&handle), sizeof(handle));
        }
    }

} // namespace KTN