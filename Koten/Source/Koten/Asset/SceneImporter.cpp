#include "ktnpch.h"
#include "SceneImporter.h"
#include "Koten/Project/Project.h"
#include "Koten/Scene/SceneSerializer.h"



namespace KTN
{
	Ref<Scene> SceneImporter::ImportScene(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
	{
		KTN_PROFILE_FUNCTION();

		if (p_Metadata.Type != AssetType::Scene)
		{
			KTN_CORE_ERROR("Invalid asset type for scene import: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		auto scene = LoadScene(p_Metadata.FilePath);

		if (scene) scene->Handle = p_Handle;

		return scene;
	}

	Ref<Scene> SceneImporter::ImportSceneFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
	{
		KTN_PROFILE_FUNCTION();

		if (p_Metadata.Type != AssetType::Scene)
		{
			KTN_CORE_ERROR("Invalid asset type for scene import: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		auto scene = CreateRef<Scene>();
		scene->Handle = p_Handle;

		SceneSerializer serializer(scene);
		if (!serializer.DeserializeBin(p_Data))
		{
			KTN_CORE_ERROR("Failed to Deserialize!");
			return nullptr;
		}

		return scene;
	}

	Ref<Scene> SceneImporter::LoadScene(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		auto scene = CreateRef<Scene>();

		SceneSerializer serializer(scene);
		if (!serializer.Deserialize(p_Path))
		{
			KTN_CORE_ERROR("Failed to Deserialize!");
			return nullptr;
		}

		return scene;
	}

	void SceneImporter::SaveScene(Ref<Scene> p_Scene, const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		SceneSerializer serializer(p_Scene);
		serializer.Serialize(p_Path);
	}

} // namespace KTN