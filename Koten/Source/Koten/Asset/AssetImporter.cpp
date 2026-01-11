#include "ktnpch.h"
#include "AssetImporter.h"

#include "SceneImporter.h"
#include "DFFontImporter.h"
#include "TextureImporter.h"
#include "PhysicsMaterial2DImporter.h"
#include "PrefabImporter.h"



namespace KTN
{
	static std::map<AssetType, std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>> s_AssetImportFunctions = {
		{ AssetType::Scene, SceneImporter::ImportScene },
		{ AssetType::Font, DFFontImporter::ImportFont },
		{ AssetType::Texture2D, TextureImporter::ImportTexture2D },
		{ AssetType::PhysicsMaterial2D, PhysicsMaterial2DImporter::ImportMaterial },
		{ AssetType::Prefab, PrefabImporter::ImportPrefab }
	};

	static std::map<AssetType, std::function<Ref<Asset>(AssetHandle, const AssetMetadata&, const Buffer&)>> s_ImportAssetFromMemoryFunctions = {
		{ AssetType::Scene, SceneImporter::ImportSceneFromMemory },
		{ AssetType::Texture2D, TextureImporter::ImportTexture2DFromMemory },
		{ AssetType::PhysicsMaterial2D, PhysicsMaterial2DImporter::ImportMaterialFromMemory },
		{ AssetType::Prefab, PrefabImporter::ImportPrefabFromMemory }
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

	Ref<Asset> AssetImporter::ImportAssetFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
	{
		KTN_PROFILE_FUNCTION();

		if (s_ImportAssetFromMemoryFunctions.find(p_Metadata.Type) == s_ImportAssetFromMemoryFunctions.end())
		{
			KTN_CORE_ERROR("No importer available for asset type: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		return s_ImportAssetFromMemoryFunctions.at(p_Metadata.Type)(p_Handle, p_Metadata, p_Data);
	}

} // namespace KTN
