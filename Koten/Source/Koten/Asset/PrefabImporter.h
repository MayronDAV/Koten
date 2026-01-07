#pragma once
#include "Asset.h"
#include "Koten/Scene/Entity.h"



namespace KTN
{
	struct PrefabContext
	{
		UUID EnttUUID;
		AssetHandle Scene;
	};

	class KTN_API Prefab : public Asset
	{
		public:
			Prefab() = default;
			~Prefab() = default;

			Entity Entt = {};
			std::string Path = "";

			ASSET_CLASS_METHODS(Prefab)
	};

	class KTN_API PrefabImporter
	{
		public:
			static Ref<Prefab> ImportPrefab(AssetHandle p_Handle, const AssetMetadata& p_Metadata);

			static Ref<Prefab> LoadPrefab(const std::string& p_Path);

			static void SavePrefab(const Ref<Prefab>& p_Prefab);
			static void SaveAsPrefab(Entity p_Entt, const std::string& p_Path);

			static void SavePrefabBin(std::ofstream& p_Out, const Ref<Prefab>& p_Prefab);
			static void SaveAsPrefabBin(std::ofstream& p_Out, Entity p_Entt, const std::string& p_Path);
			static Ref<Prefab> LoadPrefabBin(std::ifstream& p_In);
	};
} // namespace KTN