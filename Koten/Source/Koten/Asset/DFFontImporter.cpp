#include "ktnpch.h"
#include "DFFontImporter.h"



namespace KTN
{
	Ref<DFFont> DFFontImporter::ImportFont(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
	{
		KTN_PROFILE_FUNCTION();

		if (p_Metadata.Type != AssetType::Font)
		{
			KTN_CORE_ERROR("Invalid asset type for font import: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		auto fontConfig = DFFontConfig();
		if (p_Metadata.AssetData)
			fontConfig = *static_cast<DFFontConfig*>(p_Metadata.AssetData);

		return DFFont::LoadFont(p_Handle, p_Metadata.FilePath, fontConfig);
	}
}