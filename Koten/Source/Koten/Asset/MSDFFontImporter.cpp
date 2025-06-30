#include "ktnpch.h"
#include "MSDFFontImporter.h"



namespace KTN
{
	Ref<MSDFFont> MSDFFontImporter::ImportFont(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
	{
		KTN_PROFILE_FUNCTION();

		if (p_Metadata.Type != AssetType::Font)
		{
			KTN_CORE_ERROR("Invalid asset type for font import: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		auto fontConfig = MSDFFontConfig();
		if (p_Metadata.AssetData)
			fontConfig = *static_cast<MSDFFontConfig*>(p_Metadata.AssetData);

		auto font = MSDFFont::LoadFont(p_Metadata.FilePath, fontConfig);

		if (font)
			font->Handle = p_Handle;

		if (p_Metadata.AssetData)
			delete static_cast<MSDFFontConfig*>(p_Metadata.AssetData);

		return font;
	}
}