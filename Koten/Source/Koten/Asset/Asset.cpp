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

} // namespace KTN
