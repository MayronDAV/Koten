#include "ktnpch.h"
#include "PrefabImporter.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/EntitySerializer.h"
#include "AssetManager.h"
#include "Koten/Scene/SceneManager.h"
#include "Koten/Systems/B2Physics.h"

// lib
#include <yaml-cpp/yaml.h>



namespace KTN
{
	namespace
	{
		static Ref<Scene> GetScene(AssetHandle p_Handle)
		{
			KTN_PROFILE_FUNCTION_LOW();

			Ref<Scene> scene = SceneManager::GetScene(p_Handle);
			if (!scene) scene = AssetManager::Get()->GetAsset<Scene>(p_Handle);
			if (!scene)
			{
				KTN_CORE_ERROR("Failed to get scene! Please provide a valid scene handle, Handle: {}", (uint64_t)p_Handle);
				return nullptr;
			}

			return scene;
		}

		struct ReadStream
		{
			std::ifstream* InStream;
			BufferReader* Buffer;
		};

		static void SerializeEntityRecursive(YAML::Emitter& p_Out, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto& registry = p_Entity.GetScene()->GetRegistry();

			auto hcomp = p_Entity.TryGetComponent<HierarchyComponent>();

			HierarchyComponent backup{};
			bool hasHierarchy = false;

			if (hcomp)
			{
				backup = *hcomp;
				hcomp->Parent = entt::null;
				hcomp->First = entt::null;
				hcomp->Next = entt::null;
				hcomp->Prev = entt::null;
				hasHierarchy = true;
			}

			p_Out << YAML::BeginMap;

			p_Out << YAML::Key << "Entity";
			p_Out << YAML::BeginMap;

			EntitySerializer::Serialize(&p_Out, p_Entity);

			if (hasHierarchy)
				*hcomp = backup;

			p_Out << YAML::Key << "Children";
			p_Out << YAML::Value << YAML::BeginSeq;

			if (hcomp)
			{
				entt::entity child = hcomp->First;
				while (child != entt::null && registry.valid(child))
				{
					SerializeEntityRecursive(p_Out, Entity{ child, p_Entity.GetScene() });

					auto* ch = registry.try_get<HierarchyComponent>(child);
					child = ch ? ch->Next : entt::null;
				}
			}

			p_Out << YAML::EndSeq;
			p_Out << YAML::EndMap;
			p_Out << YAML::EndMap;
		}

		static Entity DeserializeEntityRecursive(const YAML::Node& p_Node, const Ref<Scene>& p_Scene, Entity p_Parent = {})
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = p_Scene->CreateEntity("Child Prefab");

			auto node = p_Node["Entity"];
			EntitySerializer::Deserialize(&node, entity);

			if (p_Parent)
				entity.SetParent(p_Parent);

			const auto& children = node["Children"];
			if (children)
			{
				for (const auto& childNode : children)
				{
					DeserializeEntityRecursive(childNode, p_Scene, entity);
				}
			}

			return entity;
		}

		static void SerializeEntityRecursive(std::ofstream& p_Out, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION_LOW();

			auto& registry = p_Entity.GetScene()->GetRegistry();

			auto hcomp = p_Entity.TryGetComponent<HierarchyComponent>();

			HierarchyComponent backup{};
			bool hasHierarchy = false;

			if (hcomp)
			{
				backup = *hcomp;
				hcomp->Parent = entt::null;
				hcomp->First = entt::null;
				hcomp->Next = entt::null;
				hcomp->Prev = entt::null;
				hasHierarchy = true;
			}

			EntitySerializer::SerializeBin(p_Out, p_Entity);

			if (hasHierarchy)
				*hcomp = backup;

			std::vector<Entity> children;

			if (hasHierarchy)
			{
				entt::entity child = backup.First;
				while (child != entt::null && registry.valid(child))
				{
					children.emplace_back(child, p_Entity.GetScene());

					auto* ch = registry.try_get<HierarchyComponent>(child);
					child = ch ? ch->Next : entt::null;
				}
			}

			uint32_t count = (uint32_t)children.size();
			p_Out.write(reinterpret_cast<const char*>(&count), sizeof(count));

			for (auto& child : children)
				SerializeEntityRecursive(p_Out, child);
		}

		static Entity DeserializeEntityRecursive(ReadStream& p_In, Scene* p_Scene, Entity p_Parent = {})
		{
			KTN_PROFILE_FUNCTION_LOW();

			Entity entity = p_Scene->CreateEntity("Child Prefab");

			if (p_In.InStream)
				EntitySerializer::DeserializeBin(*p_In.InStream, entity);
			else if (p_In.Buffer)
				EntitySerializer::DeserializeBin(*p_In.Buffer, entity);
			else return {};

			if (p_Parent)
				entity.SetParent(p_Parent);

			uint32_t childCount = 0;
			if (p_In.InStream)
				p_In.InStream->read(reinterpret_cast<char*>(&childCount), sizeof(childCount));
			else if (p_In.Buffer)
				p_In.Buffer->ReadBytes(&childCount, sizeof(childCount));

			for (uint32_t i = 0; i < childCount; i++)
			{
				DeserializeEntityRecursive(p_In, p_Scene, entity);
			}

			return entity;
		}

		static void DeserializeEntityRecursive(std::ifstream& p_In, Buffer& p_Buffer)
		{
			KTN_PROFILE_FUNCTION_LOW();

			EntitySerializer::DeserializeBin(p_In, p_Buffer);

			uint32_t childCount = 0;
			p_In.read(reinterpret_cast<char*>(&childCount), sizeof(childCount));
			p_Buffer.Write(&childCount, sizeof(childCount));

			for (uint32_t i = 0; i < childCount; i++)
			{
				DeserializeEntityRecursive(p_In, p_Buffer);
			}
		}

	} // namespace

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
		{
			prefab = CreateRef<Prefab>();

			auto scene = GetScene(ctx->Scene);
			if (!scene) return nullptr;

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
			KTN_CORE_ERROR("Failed to load file '{}'\n     Wrong extension!", p_Path);
			return nullptr;
		}

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(p_Path);
		}
		catch (const YAML::ParserException& e)
		{
			KTN_CORE_ERROR("Failed to load .ktprefab file '{}'\n     {}", p_Path, e.what());
			return nullptr;
		}

		if (!data["AssetHandle"] || !data["Root"])
		{
			KTN_CORE_ERROR("Failed to load file '{}'\n     Invalid prefab format!", p_Path);
			return nullptr;
		}

		Ref<Scene> scene = GetScene(p_SceneHandle);
		if (!scene)
			return nullptr;

		Ref<Prefab> prefab = CreateRef<Prefab>();
		prefab->Path = p_Path;
		prefab->Handle = data["AssetHandle"].as<uint64_t>();

		Entity root = DeserializeEntityRecursive(data["Root"], scene);

		prefab->Entt = root;
		return prefab;
	}

	void PrefabImporter::SavePrefab(const Ref<Prefab>& p_Prefab)
	{
		KTN_PROFILE_FUNCTION();

		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "AssetHandle" << YAML::Value << p_Prefab->Handle;

		out << YAML::Key << "Root";
		out << YAML::Value;

		SerializeEntityRecursive(out, p_Prefab->Entt);

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

		SerializeEntityRecursive(p_Out, p_Prefab->Entt);
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

		auto scene = GetScene(p_SceneHandle);
		if (!scene) return nullptr;

		auto prefab = CreateRef<Prefab>();

		AssetHandle prefabHandle;
		p_In.read(reinterpret_cast<char*>(&prefabHandle), sizeof(prefabHandle));
		prefab->Handle = prefabHandle;

		ReadStream stream = {};
		stream.InStream = &p_In;

		Entity entt = DeserializeEntityRecursive(stream, scene.get());

		prefab->Entt = entt;
		return prefab;
	}

	Ref<Prefab> PrefabImporter::LoadPrefabBin(const Buffer& p_In, AssetHandle p_SceneHandle)
	{
		KTN_PROFILE_FUNCTION();

		auto scene = GetScene(p_SceneHandle);
		if (!scene) return nullptr;

		BufferReader reader(p_In);

		auto prefab = CreateRef<Prefab>();

		AssetHandle prefabHandle;
		reader.ReadBytes(&prefabHandle, sizeof(prefabHandle));
		prefab->Handle = prefabHandle;

		ReadStream stream = {};
		stream.Buffer = &reader;

		Entity entt = DeserializeEntityRecursive(stream, scene.get());

		prefab->Entt = entt;
		return prefab;
	}

	void PrefabImporter::LoadPrefabBin(std::ifstream& p_In, Buffer& p_Buffer)
	{
		KTN_PROFILE_FUNCTION();

		AssetHandle prefabHandle;
		p_In.read(reinterpret_cast<char*>(&prefabHandle), sizeof(prefabHandle));
		p_Buffer.Write(&prefabHandle, sizeof(prefabHandle));

		DeserializeEntityRecursive(p_In, p_Buffer);
	}

}
