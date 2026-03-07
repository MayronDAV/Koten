#include "ktnpch.h"
#include "MaterialImporter.h"

// lib
#include <yaml-cpp/yaml.h>



namespace KTN
{
    Ref<Material> MaterialImporter::ImportMaterial(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::Material)
        {
            KTN_CORE_ERROR("Invalid asset type for Material import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        Ref<Material> material = nullptr;
        if (!p_Metadata.FilePath.empty())
            material = LoadMaterial(p_Metadata.FilePath);
        else
            material = CreateRef<Material>();

        if (material) material->Handle = p_Handle;
        return material;
    }

    Ref<Material> MaterialImporter::ImportMaterialFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::Material)
        {
            KTN_CORE_ERROR("Invalid asset type for Material import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        BufferReader reader(p_Data);

        Ref<Material> material = CreateRef<Material>();
        material->Handle = p_Handle;

        material->DeserializeBin(reader);

        return material;
    }

    Ref<Material> MaterialImporter::LoadMaterial(const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        if (FileSystem::GetExtension(p_Path) != ".ktmat")
        {
            KTN_CORE_ERROR("Invalid file extension, it should be .ktmat!");
            return nullptr;
        }

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(p_Path);
        }
        catch (YAML::ParserException e)
        {
            KTN_CORE_ERROR("Failed to load .ktmat file '{0}'\n     {1}", p_Path, e.what());
            return nullptr;
        }

        auto material                            = CreateRef<Material>();
        if (!data["Material"])
            return nullptr;

        auto handle                              = data["Material"].as<AssetHandle>();
        if (handle != 0)
            material->Handle                     = handle;

        if (data["Name"]) material->Name         = data["Name"].as<std::string>();
        if (data["Color"]) material->AlbedoColor = data["Color"].as<glm::vec4>();
        if (data["Texture"]) material->Texture   = data["Texture"].as<uint64_t>();

        return material;
    }

} // namespace KTN
