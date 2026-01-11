#include "ktnpch.h"
#include "PrefabImporter.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/EntitySerializer.h"
#include "AssetManager.h"

// lib
#include <yaml-cpp/yaml.h>



namespace KTN
{
	Ref<Prefab> PrefabImporter::ImportPrefab(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
	{
		KTN_PROFILE_FUNCTION();

		if (p_Metadata.Type != AssetType::Prefab)
		{
			KTN_CORE_ERROR("Invalid asset type for prefab import: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		auto ctx = static_cast<PrefabContext*>(p_Metadata.AssetData);

		Ref<Prefab> prefab = nullptr;
		if (FileSystem::Exists(p_Metadata.FilePath))
			prefab = LoadPrefab(ctx->Scene, p_Metadata.FilePath);
		else
			prefab = CreateRef<Prefab>();

		if (!prefab)
		{
			Ref<Scene> scene = AssetManager::Get()->GetAsset<Scene>(ctx->Scene);
			prefab->Entt = scene->GetEntityByUUID(ctx->EnttUUID);
			prefab->Handle = p_Handle;
		}

		prefab->Path = p_Metadata.FilePath;
		return prefab;
	}

	Ref<Prefab> PrefabImporter::ImportPrefabFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
	{
		KTN_PROFILE_FUNCTION();

		if (p_Metadata.Type != AssetType::Prefab)
		{
			KTN_CORE_ERROR("Invalid asset type for prefab import: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		auto ctx = static_cast<PrefabContext*>(p_Metadata.AssetData);

		auto prefab = LoadPrefabBin(p_Data, ctx->Scene);
		prefab->Path = p_Metadata.FilePath;

		return prefab;
	}

	Ref<Prefab> PrefabImporter::LoadPrefab(AssetHandle p_SceneHandle, const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		if (FileSystem::GetExtension(p_Path) != ".ktprefab")
		{
			KTN_CORE_ERROR("Failed to load file '{0}'\n     Wrong extension!", p_Path);
			return nullptr;
		}

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(p_Path);
		}
		catch (YAML::ParserException e)
		{
			KTN_CORE_ERROR("Failed to load .ktprefab file '{0}'\n     {1}", p_Path, e.what());
			return nullptr;
		}

		Ref<Prefab> prefab = CreateRef<Prefab>();
		prefab->Path = p_Path;

		if (!data["AssetHandle"])
		{
			KTN_CORE_ERROR("Failed to load file '{0}'\n     The AssetHandle data doesn't exist!", p_Path);
			return nullptr;
		}	
		prefab->Handle = data["AssetHandle"].as<uint64_t>();

		Ref<Scene> scene = AssetManager::Get()->GetAsset<Scene>(p_SceneHandle);
		Entity deserializedEntt = scene->CreateEntity(UUID(), "Entity Prefab");

		EntitySerializer::Deserialize(&data, deserializedEntt);

		auto& tag = deserializedEntt.GetComponent<TagComponent>().Tag;
		tag = tag + " (Prefab)";

		auto comp = deserializedEntt.TryGetComponent<TransformComponent>();
		if (comp)
		{
			comp->SetLocalTranslation({ 0.0f, 0.0f, 0.0f });
			comp->SetLocalRotation({ 0.0f, 0.0f, 0.0f });
		}

		prefab->Entt = deserializedEntt;

		return prefab;
	}

	void PrefabImporter::SavePrefab(const Ref<Prefab>& p_Prefab)
	{
		KTN_PROFILE_FUNCTION();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "AssetHandle" << YAML::Value << p_Prefab->Handle;
		EntitySerializer::Serialize(&out, p_Prefab->Entt);
		out << YAML::EndMap;

		std::ofstream fout(p_Prefab->Path);
		fout << out.c_str();

		KTN_CORE_INFO("Prefab serialized to path: {}", p_Prefab->Path);
	}

	void PrefabImporter::SaveAsPrefab(Entity p_Entt, const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		auto prefab = CreateRef<Prefab>();
		prefab->Handle = AssetHandle();
		prefab->Path = p_Path;
		prefab->Entt = p_Entt;

		SavePrefab(prefab);
	}

	void PrefabImporter::SavePrefabBin(std::ofstream& p_Out, const Ref<Prefab>& p_Prefab)
	{
		KTN_PROFILE_FUNCTION();

		p_Out.write(reinterpret_cast<const char*>(&p_Prefab->Handle), sizeof(AssetHandle));

		EntitySerializer::SerializeBin(p_Out, p_Prefab->Entt);
	}

	void PrefabImporter::SaveAsPrefabBin(std::ofstream& p_Out, Entity p_Entt, const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		auto prefab = CreateRef<Prefab>();
		prefab->Handle = AssetHandle();
		prefab->Path = p_Path;
		prefab->Entt = p_Entt;

		SavePrefabBin(p_Out, prefab);
	}

	Ref<Prefab> PrefabImporter::LoadPrefabBin(std::ifstream& p_In, AssetHandle p_SceneHandle)
	{
		KTN_PROFILE_FUNCTION();

		auto prefab = CreateRef<Prefab>();

		AssetHandle prefabHandle;
		p_In.read(reinterpret_cast<char*>(&prefabHandle), sizeof(prefabHandle));
		prefab->Handle = prefabHandle;

		Ref<Scene> scene = AssetManager::Get()->GetAsset<Scene>(p_SceneHandle);
		Entity entt = scene->CreateEntity(UUID());

		EntitySerializer::DeserializeBin(p_In, entt);

		auto& tag = entt.GetComponent<TagComponent>().Tag;
		tag = tag + " (Prefab)";

		auto comp = entt.TryGetComponent<TransformComponent>();
		if (comp)
		{
			comp->SetLocalTranslation({ 0.0f, 0.0f, 0.0f });
			comp->SetLocalRotation({ 0.0f, 0.0f, 0.0f });
		}

		prefab->Entt = entt;
		return prefab;
	}

	Ref<Prefab> PrefabImporter::LoadPrefabBin(const Buffer& p_In, AssetHandle p_SceneHandle)
	{
		KTN_PROFILE_FUNCTION();

		BufferReader reader(p_In);

		auto prefab = CreateRef<Prefab>();

		AssetHandle prefabHandle;
		reader.ReadBytes(&prefabHandle, sizeof(prefabHandle));
		prefab->Handle = prefabHandle;

		Ref<Scene> scene = AssetManager::Get()->GetAsset<Scene>(p_SceneHandle);
		Entity entt = scene->CreateEntity(UUID());

		EntitySerializer::DeserializeBin(reader, entt);

		auto& tag = entt.GetComponent<TagComponent>().Tag;
		tag = tag + " (Prefab)";

		auto comp = entt.TryGetComponent<TransformComponent>();
		if (comp)
		{
			comp->SetLocalTranslation({ 0.0f, 0.0f, 0.0f });
			comp->SetLocalRotation({ 0.0f, 0.0f, 0.0f });
		}

		prefab->Entt = entt;
		return prefab;
	}

	void PrefabImporter::LoadPrefabBin(std::ifstream& p_In, Buffer& p_Buffer)
	{
		KTN_PROFILE_FUNCTION();

		AssetHandle prefabHandle;
		p_In.read(reinterpret_cast<char*>(&prefabHandle), sizeof(prefabHandle));
		p_Buffer.Write(&prefabHandle, sizeof(prefabHandle));

		EntitySerializer::DeserializeBin(p_In, p_Buffer);
	}

}
