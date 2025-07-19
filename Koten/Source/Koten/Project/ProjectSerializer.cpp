#include "ktnpch.h"
#include "ProjectSerializer.h"
#include "Koten/Utils/Utils.h"

// lib
#include <yaml-cpp/yaml.h>

// std
#include <fstream>




namespace KTN
{
	#define ADD_KEY_VALUE(key, value) out << YAML::Key << key << YAML::Value << value

	bool ProjectSerializer::Serialize(const std::filesystem::path& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		const auto& config = m_Project->GetConfig();

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "Project" << YAML::Value;
			{
				out << YAML::BeginMap;// Project
				ADD_KEY_VALUE("Name", config.Name);
				ADD_KEY_VALUE("StartScene", config.StartScene);
				ADD_KEY_VALUE("AssetDirectory", config.AssetDirectory.string());
				out << YAML::EndMap; // Project
			}
			out << YAML::EndMap; // Root
		}

		std::ofstream fout(p_Path);
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		auto& config = m_Project->GetConfig();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(p_Path.string());
		}
		catch (YAML::ParserException e)
		{
			KTN_CORE_ERROR("Failed to load project file '{0}'\n     {1}", p_Path.string(), e.what());
			return false;
		}

		auto projectNode = data["Project"];
		if (!projectNode)
			return false;

		config.Name = projectNode["Name"].as<std::string>();
		config.StartScene = projectNode["StartScene"].as<AssetHandle>();
		config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		return true;
	}

	struct ProjectHeader
	{
		char Magic[4] = { 'K', 'T', 'D', 'T' };
		uint32_t Version = 1;

		AssetHandle StartScene = 0;
	};

	bool ProjectSerializer::SerializeRuntime(const std::filesystem::path& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		const auto& config = m_Project->GetConfig();

		std::ofstream out(p_Path, std::ios::binary | std::ios::trunc);

		ProjectHeader header = {};
		header.StartScene = config.StartScene;
		out.write(reinterpret_cast<const char*>(&header), sizeof(header));

		Utils::WriteString(out, config.Name);
		Utils::WriteString(out, config.AssetDirectory.string());
		Utils::WriteString(out, config.IconPath);

		return true;
	}

	bool ProjectSerializer::DeserializeRuntime(const std::filesystem::path& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		auto& config = m_Project->GetConfig();

		std::ifstream in(p_Path, std::ios::binary);
		if (!in)
		{
			KTN_CORE_ERROR("Failed to open project runtime file: {}", p_Path.string());
			return false;
		}

		ProjectHeader header = {};
		in.read(reinterpret_cast<char*>(&header), sizeof(header));
		if (memcmp(header.Magic, "KTDT", 4) != 0 || header.Version != 1)
		{
			KTN_CORE_ERROR("Invalid project runtime file format: {}", p_Path.string());
			return false;
		}

		config.StartScene = header.StartScene;
		config.Name = Utils::ReadString(in);
		config.AssetDirectory = Utils::ReadString(in);
		config.IconPath = Utils::ReadString(in);

		return true;
	}

} // namespace KTN
