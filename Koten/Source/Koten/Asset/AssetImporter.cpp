#include "ktnpch.h"
#include "AssetImporter.h"

#include "SceneImporter.h"
#include "TextureImporter.h"



namespace KTN
{
	using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
	static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
		{ AssetType::Scene, SceneImporter::ImportScene },
		{ AssetType::Texture2D, TextureImporter::ImportTexture2D }
	};

	Ref<Asset> AssetImporter::ImportAsset(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
	{
		KTN_PROFILE_FUNCTION();

		if (s_AssetImportFunctions.find(p_Metadata.Type) == s_AssetImportFunctions.end())
		{
			KTN_CORE_ERROR("No importer available for asset type: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		return s_AssetImportFunctions.at(p_Metadata.Type)(p_Handle, p_Metadata);
	}

} // namespace KTN
