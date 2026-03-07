#pragma once
#include "Asset.h"
#include "Koten/Graphics/Material.h"



namespace KTN
{
    struct KTN_API MaterialImporter
    {
        static Ref<Material> ImportMaterial(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
        static Ref<Material> ImportMaterialFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data);

        static Ref<Material> LoadMaterial(const std::string& p_Path);
    };

} // namespace KTN
