#pragma once
#include "Asset.h"
#include "Koten/Graphics/Material.h"



namespace KTN
{
    struct KTN_API MaterialImporter
    {
        static Ref<Material> Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
        static Ref<Material> ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data);

        static Ref<Material> Load(const std::string& p_Path);
    };

} // namespace KTN
