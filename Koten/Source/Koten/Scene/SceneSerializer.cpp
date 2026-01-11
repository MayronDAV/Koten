#include "ktnpch.h"
#include "SceneSerializer.h"
#include "EntitySerializer.h"

// lib
#include <yaml-cpp/yaml.h>




namespace KTN
{
	SceneSerializer::SceneSerializer(const Ref<Scene>& p_Scene)
		: m_Scene(p_Scene)
	{
	}

	void SceneSerializer::Serialize(const std::string& p_Filepath)
	{
		KTN_PROFILE_FUNCTION();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << m_Scene->Handle;
		out << YAML::Key << "Configs" << YAML::Value << YAML::BeginSeq;
		{
			out << YAML::BeginMap;
			auto& config = m_Scene->GetConfig();
			out << YAML::Key << "UsePhysics2D" << YAML::Value << config.UsePhysics2D;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		for (auto entity : m_Scene->GetRegistry().view<entt::entity>())
		{
			auto entt = Entity{ entity, m_Scene.get() };
			if (!entt)
				continue;

			out << YAML::BeginMap;
			out << YAML::Key << "Entity" << YAML::Value << entt.GetUUID();

			EntitySerializer::Serialize(&out, entt);

			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(p_Filepath);
		fout << out.c_str();

		KTN_CORE_INFO("Scene serialized to path: {}", p_Filepath);
	}

	bool SceneSerializer::Deserialize(const std::string& p_Filepath)
	{
		KTN_PROFILE_FUNCTION();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(p_Filepath);
		}
		catch (YAML::ParserException e)
		{
			KTN_CORE_ERROR("Failed to load .uyscene file '{0}'\n     {1}", p_Filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		auto sceneID = data["Scene"].as<AssetHandle>();
		KTN_CORE_INFO("Deserializing scene: (filename: '{}', id: '{}')", FileSystem::GetName(p_Filepath), (uint64_t)sceneID);
		if (sceneID != 0)
			m_Scene->Handle = sceneID;

		auto configs = data["Configs"];
		if (configs)
		{
			auto& config = m_Scene->GetConfig();
			for (auto value : configs)
			{
				config.UsePhysics2D = value["UsePhysics2D"].as<bool>();
			}
		}

		auto entts = data["Entities"];
		if (entts)
		{
			// TODO: Maybe find another way to load the entt hierarchy
			for (auto entt : entts)
			{
				auto enttID = entt["Entity"].as<uint64_t>();
				Entity deserializedEntt = m_Scene->CreateEntity((UUID)enttID, "Entity");
			}

			for (auto entt : entts)
			{
				auto enttID = entt["Entity"].as<uint64_t>();
				Entity deserializedEntt = m_Scene->GetEntityByUUID((UUID)enttID);

				EntitySerializer::Deserialize(&entt, deserializedEntt);
			}
		}

		return true;
	}

	void SceneSerializer::SerializeBin(std::ofstream& p_Out)
	{
		KTN_PROFILE_FUNCTION();

		const auto& enttMap = m_Scene->GetEntityMap();

		SceneConfig config = m_Scene->GetConfig();
		p_Out.write(reinterpret_cast<const char*>(&config), sizeof(config));

		auto size = (int)enttMap.size();
		p_Out.write(reinterpret_cast<const char*>(&size), sizeof(size));

		for (const auto& [uuid, entity] : enttMap)
		{
			p_Out.write(reinterpret_cast<const char*>(&uuid), sizeof(uuid));

			auto entt = Entity{ entity, m_Scene.get() };
			EntitySerializer::SerializeBin(p_Out, entt);
		}
	}

	bool SceneSerializer::DeserializeBin(std::ifstream& p_In)
	{
		KTN_PROFILE_FUNCTION();

		SceneConfig config;
		p_In.read(reinterpret_cast<char*>(&config), sizeof(config));
		m_Scene->GetConfig() = config;

		int size = 0;
		p_In.read(reinterpret_cast<char*>(&size), sizeof(size));

		for (int i = 0; i < size; i++)
		{
			UUID uuid;
			p_In.read(reinterpret_cast<char*>(&uuid), sizeof(uuid));

			Entity entt = m_Scene->CreateEntity(uuid);
			EntitySerializer::DeserializeBin(p_In, entt);
		}

		return true;
	}

	bool SceneSerializer::DeserializeBin(const Buffer& p_Buffer)
	{
		KTN_PROFILE_FUNCTION();

		BufferReader reader(p_Buffer);

		SceneConfig config;
		reader.ReadBytes(&config, sizeof(config));
		m_Scene->GetConfig() = config;

		int size = 0;
		reader.ReadBytes(&size, sizeof(size));

		for (int i = 0; i < size; i++)
		{
			UUID uuid;
			reader.ReadBytes(&uuid, sizeof(uuid));

			Entity entt = m_Scene->CreateEntity(uuid);
			EntitySerializer::DeserializeBin(reader, entt);
		}

		return true;
	}

	bool SceneSerializer::DeserializeBin(std::ifstream& p_In, Buffer& p_Buffer)
	{
		KTN_PROFILE_FUNCTION();

		SceneConfig config;
		p_In.read(reinterpret_cast<char*>(&config), sizeof(config));
		p_Buffer.Write(&config, sizeof(config));

		int size = 0;
		p_In.read(reinterpret_cast<char*>(&size), sizeof(size));
		p_Buffer.Write(&size, sizeof(size));

		for (int i = 0; i < size; i++)
		{
			UUID uuid;
			p_In.read(reinterpret_cast<char*>(&uuid), sizeof(uuid));
			p_Buffer.Write(&uuid, sizeof(uuid));

			EntitySerializer::DeserializeBin(p_In, p_Buffer);
		}

		return true;
	}

} // namespace KTN
