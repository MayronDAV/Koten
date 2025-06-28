#include "ktnpch.h"
#include "AssetManager.h"
#include "AssetImporter.h"
#include "Koten/Project/Project.h"

// lib
#include <yaml-cpp/yaml.h>

// std
#include <fstream>
#include <algorithm>



namespace YAML {

	template<>
	struct convert<KTN::UUID>
	{
		static Node encode(const KTN::UUID& p_UUID)
		{
			Node node;
			node.push_back((uint64_t)p_UUID);
			return node;
		}

		static bool decode(const Node& p_Node, KTN::UUID& p_UUID)
		{
			p_UUID = p_Node.as<uint64_t>();
			return true;
		}
	};

	Emitter& operator<<(Emitter& p_Out, const KTN::AssetHandle& p_Handle)
	{
		p_Out << (uint64_t)p_Handle;
		return p_Out;
	}
}


namespace KTN
{
	Ref<Asset> AssetManager::GetAsset(AssetHandle p_Handle)
	{
		KTN_PROFILE_FUNCTION();

		if (!IsAssetHandleValid(p_Handle))
			return nullptr;

		if (IsAssetLoaded(p_Handle))
			return m_LoadedAssets.at(p_Handle);

		if (m_Config.LoadAssetsFromPath)
		{
			const auto& metadata = GetMetadata(p_Handle);
			auto asset = AssetImporter::ImportAsset(p_Handle, metadata);
			if (!asset)
			{
				KTN_CORE_ERROR("AssetManager::GetAsset - Asset import failed!");
				return nullptr;
			}

			m_LoadedAssets[p_Handle] = asset;
			return asset;
		}

		KTN_CORE_ERROR("AssetManager::GetAsset - Something wrong!");
		return nullptr;
	}

	AssetHandle AssetManager::GetHandleByPath(const std::string& p_FilePath) const
	{
		KTN_PROFILE_FUNCTION();

		auto it = std::find_if(m_AssetRegistry.begin(), m_AssetRegistry.end(),
			[&p_FilePath](const auto& p_Pair) 
			{
				return p_Pair.second.FilePath == p_FilePath;
			}
		);

		return (it != m_AssetRegistry.end()) ? it->first : (AssetHandle)0;
	}

	AssetType AssetManager::GetAssetType(AssetHandle p_Handle) const
	{
		KTN_PROFILE_FUNCTION();

		return GetMetadata(p_Handle).Type;
	}

	AssetHandle AssetManager::ImportAsset(AssetType p_Type, const std::string& p_FilePath)
	{
		KTN_PROFILE_FUNCTION();

		if (auto handle = GetHandleByPath(p_FilePath); IsAssetHandleValid(handle))
		{
			return handle;
		}

		AssetHandle handle; // generate new handle
		AssetMetadata metadata;
		metadata.FilePath = FileSystem::GetRelative(p_FilePath, Project::GetAssetDirectory().string());
		metadata.Type = p_Type;
		Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
		if (asset)
		{
			asset->Handle = handle;
			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = metadata;
			SerializeAssetRegistry();
			return handle;
		}

		KTN_CORE_ERROR("Failed to import asset: {}, {}", GetAssetTypeName(p_Type), p_FilePath);
		return 0;
	}

	bool AssetManager::IsAssetHandleValid(AssetHandle p_Handle) const
	{
		KTN_PROFILE_FUNCTION();

		return p_Handle != 0 && m_AssetRegistry.find(p_Handle) != m_AssetRegistry.end();
	}

	bool AssetManager::IsAssetLoaded(AssetHandle p_Handle) const
	{
		KTN_PROFILE_FUNCTION();

		return p_Handle != 0 && m_LoadedAssets.find(p_Handle) != m_LoadedAssets.end();
	}

	const AssetMetadata& AssetManager::GetMetadata(AssetHandle p_Handle) const
	{
		KTN_PROFILE_FUNCTION();

		static AssetMetadata s_EmptyMetadata;
		if (!IsAssetHandleValid(p_Handle))
			return s_EmptyMetadata;

		return m_AssetRegistry.at(p_Handle);
	}

	void AssetManager::SerializeAssetRegistry()
	{
		KTN_PROFILE_FUNCTION();

		auto path = Project::GetAssetDirectory() / "AssetRegistry.ktreg";

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "AssetRegistry" << YAML::Value;

			out << YAML::BeginSeq;
			for (const auto& [handle, metadata] : m_AssetRegistry)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;
				out << YAML::Key << "FilePath" << YAML::Value << metadata.FilePath;
				out << YAML::Key << "Type" << YAML::Value << (std::string)GetAssetTypeName(metadata.Type);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap; // Root
		}

		std::ofstream fout(path);
		fout << out.c_str();
	}

	bool AssetManager::DeserializeAssetRegistry()
	{
		KTN_PROFILE_FUNCTION();

		auto path = Project::GetAssetDirectory() / "AssetRegistry.ktreg";
		if (!FileSystem::Exists(path.string()))
			return false;

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e)
		{
			KTN_CORE_ERROR("Failed to load asset registry file '{0}'\n     {1}", path.string(), e.what());
			return false;
		}

		auto rootNode = data["AssetRegistry"];
		if (!rootNode)
			return false;

		for (const auto& node : rootNode)
		{
			AssetHandle handle = node["Handle"].as<uint64_t>();
			auto& metadata = m_AssetRegistry[handle];
			metadata.FilePath = (Project::GetAssetDirectory() / node["FilePath"].as<std::string>()).string();
			auto type = node["Type"].as<std::string>();
			metadata.Type = GetAssetTypeFromName(type.c_str());
		}

		return true;
	}


} // namespace KTN
