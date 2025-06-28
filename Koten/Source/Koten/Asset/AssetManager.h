#pragma once
#include "Asset.h"

// std
#include <map>



namespace KTN
{
	using AssetMap = std::map<AssetHandle, Ref<Asset>>;
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>;

	struct AssetManagerConfig
	{
		bool LoadAssetsFromPath = true;

		// TODO: GenAssetPack and LoadAssetsFromAssetPack
	};

	class KTN_API AssetManager
	{
		public:
			AssetHandle ImportAsset(AssetType p_Type, const std::string& p_FilePath);

			bool IsAssetHandleValid(AssetHandle p_Handle) const;
			bool IsAssetLoaded(AssetHandle p_Handle) const;

			Ref<Asset> GetAsset(AssetHandle p_Handle);
			AssetHandle GetHandleByPath(const std::string& p_FilePath) const;
			AssetType GetAssetType(AssetHandle p_Handle) const;
			const AssetMetadata& GetMetadata(AssetHandle p_Handle) const;
			AssetManagerConfig& GetConfig() { return m_Config; }

		private:
			AssetRegistry m_AssetRegistry;
			AssetMap m_LoadedAssets;

			AssetManagerConfig m_Config = {};
	};


} // namespace KTN
