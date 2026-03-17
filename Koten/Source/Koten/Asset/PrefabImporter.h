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
            static Ref<Prefab> Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
            static Ref<Prefab> ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data);

            static Ref<Prefab> Load(AssetHandle p_SceneHandle, const std::string& p_Path);

            static void Save(const Ref<Prefab>& p_Prefab);
            static void SaveAs(Entity p_Entt, const std::string& p_Path);

            static void SaveBin(std::ofstream& p_Out, const Ref<Prefab>& p_Prefab);
            static void SaveAsBin(std::ofstream& p_Out, Entity p_Entt, const std::string& p_Path);
            static Ref<Prefab> LoadBin(std::ifstream& p_In, AssetHandle p_SceneHandle);
            static Ref<Prefab> LoadBin(const Buffer& p_In, AssetHandle p_SceneHandle);

            static void LoadBin(std::ifstream& p_In, Buffer& p_Buffer);
    };
} // namespace KTN