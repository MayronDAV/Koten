#include "ktnpch.h"
#include "Asset.h"

// lib
#include <magic_enum/magic_enum.hpp>



namespace KTN
{
    const char* GetAssetTypeName(AssetType p_Type)
    {
        auto ret = magic_enum::enum_name(p_Type);
        return ret.data();
    }

    AssetType GetAssetTypeFromName(const char* p_Name)
    {
        auto type = magic_enum::enum_cast<AssetType>(p_Name);
        return type.has_value() ? type.value() : AssetType::None;
    }

    const char* GetAssetScopeName(AssetScope p_Scope)
    {
        auto ret = magic_enum::enum_name(p_Scope);
        return ret.data();
    }

    AssetScope GetAssetScopeFromName(const char* p_Name)
    {
        auto type = magic_enum::enum_cast<AssetScope>(p_Name);
        return type.has_value() ? type.value() : AssetScope::Project;
    }

} // namespace KTN
