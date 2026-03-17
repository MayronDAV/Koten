#pragma once
#include "Asset.h"
#include "Koten/Scene/Scene.h"



namespace KTN
{
    class KTN_API SceneImporter
    {
        public:
            static Ref<Scene> Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
            static Ref<Scene> ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data);

            static Ref<Scene> Load(const std::string& p_Path);

            static void Save(Ref<Scene> p_Scene, const std::string& p_Path);
    };
} // namespace KTN