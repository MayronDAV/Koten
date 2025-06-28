#include "ktnpch.h"
#include "AssetManager.h"
#include "AssetImporter.h"

// std
#include <algorithm>



namespace KTN
{
	Ref<Asset> AssetManager::GetAsset(AssetHandle p_Handle)
	{
		KTN_PROFILE_FUNCTION();

		if (!IsAssetHandleValid(p_Handle))
			return nullptr;

		if (IsAssetLoaded(p_Handle))
			return m_LoadedAssets.at(p_Handle);

		if (m_Config.LoadAssetsFromPath)
		{
			const auto& metadata = GetMetadata(p_Handle);
			auto asset = AssetImporter::ImportAsset(p_Handle, metadata);
			if (!asset)
				KTN_CORE_ERROR("AssetManager::GetAsset - Asset import failed!");
			return asset;
		}

		KTN_CORE_ERROR("AssetManager::GetAsset - Something wrong!");
		return nullptr;
	}

	AssetHandle AssetManager::GetHandleByPath(const std::string& p_FilePath) const
	{
		KTN_PROFILE_FUNCTION();

		auto it = std::find_if(m_AssetRegistry.begin(), m_AssetRegistry.end(),
			[&p_FilePath](const auto& p_Pair) 
			{
				return p_Pair.second.FilePath == p_FilePath;
			}
		);

		return (it != m_AssetRegistry.end()) ? it->first : (AssetHandle)0;
	}

	AssetType AssetManager::GetAssetType(AssetHandle p_Handle) const
	{
		KTN_PROFILE_FUNCTION();

		return GetMetadata(p_Handle).Type;
	}

	AssetHandle AssetManager::ImportAsset(AssetType p_Type, const std::string& p_FilePath)
	{
		KTN_PROFILE_FUNCTION();

		AssetHandle handle; // generate new handle
		AssetMetadata metadata;
		metadata.FilePath = p_FilePath;
		metadata.Type = p_Type;
		Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
		if (asset)
		{
			asset->Handle = handle;
			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = metadata;
			// TODO: Serialize asset registry
			return handle;
		}

		KTN_CORE_ERROR("Failed to import asset: {}, {}", GetAssetTypeName(p_Type), p_FilePath);
		return 0;
	}

	bool AssetManager::IsAssetHandleValid(AssetHandle p_Handle) const
	{
		KTN_PROFILE_FUNCTION();

		return p_Handle != 0 && m_AssetRegistry.find(p_Handle) != m_AssetRegistry.end();
	}

	bool AssetManager::IsAssetLoaded(AssetHandle p_Handle) const
	{
		KTN_PROFILE_FUNCTION();

		return p_Handle != 0 && m_LoadedAssets.find(p_Handle) != m_LoadedAssets.end();
	}

	const AssetMetadata& AssetManager::GetMetadata(AssetHandle p_Handle) const
	{
		KTN_PROFILE_FUNCTION();

		if (!IsAssetHandleValid(p_Handle))
			return {};

		return m_AssetRegistry.at(p_Handle);
	}


} // namespace KTN
