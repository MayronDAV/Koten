#pragma once
#include "Asset.h"
#include "Koten/Physics/PhysicsMaterial2D.h"



namespace KTN
{
    struct KTN_API PhysicsMaterial2DImporter
    {
        static Ref<PhysicsMaterial2D> Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
        static Ref<PhysicsMaterial2D> ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data);

        static Ref<PhysicsMaterial2D> Load(const std::string& p_Path);
    };

} // namespace KTN
