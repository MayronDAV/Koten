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

		Ref<Prefab> prefab = nullptr;
		if (FileSystem::Exists(p_Metadata.FilePath))
			prefab = LoadPrefab(p_Metadata.FilePath);
		else
			prefab = CreateRef<Prefab>();

		if (prefab)
		{
			auto ctx = static_cast<PrefabContext*>(p_Metadata.AssetData);
			Ref<Scene> scene = As<Asset, Scene>(AssetManager::Get()->GetAsset(ctx->Scene));
			prefab->Entt = scene->GetEntityByUUID(ctx->EnttUUID);
			prefab->Path = p_Metadata.FilePath;
			prefab->Handle = p_Handle;
		}

		return prefab;
	}

	Ref<Prefab> PrefabImporter::LoadPrefab(const std::string& p_Path)
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

		if (!data["Scene"])
		{
			KTN_CORE_ERROR("Failed to load file '{0}'\n     The Scene data doesn't exist!", p_Path);
			return nullptr;
		}
		auto sceneID = data["Scene"].as<AssetHandle>();
		
		Ref<Scene> scene = As<Asset, Scene>(AssetManager::Get()->GetAsset(sceneID));
		if (!scene)
		{
			KTN_CORE_ERROR("Failed to load file '{0}'\n     Failed to get Scene asset!", p_Path);
			return nullptr;
		}

		if (!data["AssetHandle"])
		{
			KTN_CORE_ERROR("Failed to load file '{0}'\n     The AssetHandle data doesn't exist!", p_Path);
			return nullptr;
		}	
		prefab->Handle = data["AssetHandle"].as<uint64_t>();

		if (!data["Entity"])
		{
			KTN_CORE_ERROR("Failed to load file '{0}'\n     The Entity data doesn't exist!", p_Path);
			return nullptr;
		}
		auto enttID = data["Entity"].as<uint64_t>();

		Entity deserializedEntt = scene->GetEntityByUUID((UUID)enttID);
		EntitySerializer::Deserialize(&data, deserializedEntt);

		prefab->Entt = deserializedEntt;

		return prefab;
	}

	void PrefabImporter::SavePrefab(const Ref<Prefab>& p_Prefab)
	{
		KTN_PROFILE_FUNCTION();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << p_Prefab->Entt.GetScene()->Handle;
		out << YAML::Key << "AssetHandle" << YAML::Value << p_Prefab->Handle;
		out << YAML::Key << "Entity" << YAML::Value << p_Prefab->Entt.GetUUID();
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

		p_Out.write(reinterpret_cast<const char*>(&p_Prefab->Entt.GetScene()->Handle), sizeof(AssetHandle));

		auto uuid = p_Prefab->Entt.GetUUID();
		p_Out.write(reinterpret_cast<const char*>(&uuid), sizeof(UUID));
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

	Ref<Prefab> PrefabImporter::LoadPrefabBin(std::ifstream& p_In)
	{
		KTN_PROFILE_FUNCTION();

		auto prefab = CreateRef<Prefab>();

		AssetHandle sceneHandle;
		p_In.read(reinterpret_cast<char*>(&sceneHandle), sizeof(AssetHandle));
		Ref<Scene> scene = As<Asset, Scene>(AssetManager::Get()->GetAsset(sceneHandle));

		UUID uuid;
		p_In.read(reinterpret_cast<char*>(&uuid), sizeof(uuid));

		Entity entt = scene->CreateEntity(uuid);
		EntitySerializer::DeserializeBin(p_In, entt);

		return prefab;
	}

}
