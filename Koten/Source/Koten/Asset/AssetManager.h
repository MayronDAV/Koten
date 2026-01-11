#pragma once
#include "Asset.h"
#include "Koten/Core/Buffer.h"

// std
#include <map>



namespace KTN
{
	using AssetMap = std::map<AssetHandle, Ref<Asset>>;
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>;
	using AssetCache = std::map<AssetHandle, Ref<ScopedBuffer>>;

	struct AssetManagerConfig
	{
		bool LoadAssetsFromPath = true;
		bool LoadAssetsFromMemory = false;
	};

	class KTN_API AssetManager
	{
		public:
			static Ref<AssetManager> Create(const AssetManagerConfig& p_Config = {});

			static Ref<AssetManager> Get() { return s_Instance; }

		public:
			~AssetManager();

			AssetHandle ImportAsset(AssetType p_Type, const std::string& p_FilePath, bool p_Force = false);
			AssetHandle ImportAsset(const AssetMetadata& p_Metadata, bool p_Force = false);
			bool ImportAsset(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Ref<Asset>& p_Asset = nullptr);

			bool IsAssetHandleValid(AssetHandle p_Handle) const;
			bool IsAssetLoaded(AssetHandle p_Handle) const;
			bool HasAsset(AssetType p_Type, const std::string& p_FilePath) const;

			template<typename T>
			Ref<T> GetAsset(AssetHandle p_Handle)
			{
				if constexpr (std::is_same_v<Asset, T>)
					return GetAsset(p_Handle);

				return As<Asset, T>(GetAsset(p_Handle));
			}

			bool RemoveAsset(AssetHandle p_Handle);

			AssetHandle GetHandleByPath(const std::string& p_FilePath) const;
			AssetType GetAssetType(AssetHandle p_Handle) const;
			const AssetMetadata& GetMetadata(AssetHandle p_Handle) const;
			AssetMetadata& GetMetadata(AssetHandle p_Handle);
			const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }

			AssetManagerConfig& GetConfig() { return m_Config; }

			void SerializeAssetPack(const std::filesystem::path& p_Folder = "Assets");
			bool DeserializeAssetPack(const std::filesystem::path& p_Folder = "Assets");

			void SerializeAssetRegistry();
			bool DeserializeAssetRegistry();

		private:
			Ref<Asset> GetAsset(AssetHandle p_Handle);

		private:
			AssetRegistry m_AssetRegistry;
			AssetMap m_LoadedAssets;
			AssetCache m_AssetCache;

			bool m_IsLoadedAssetPack = false;
			bool m_NeedsToUpdate = false;

			AssetManagerConfig m_Config = {};

			inline static Ref<AssetManager> s_Instance = nullptr;
	};


} // namespace KTN
