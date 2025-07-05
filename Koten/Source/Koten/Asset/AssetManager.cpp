#include "ktnpch.h"
#include "AssetManager.h"
#include "AssetImporter.h"
#include "Koten/Project/Project.h"
#include "Koten/Graphics/MSDFFont.h"

// lib
#include <yaml-cpp/yaml.h>
#include <magic_enum/magic_enum.hpp>

// std
#include <fstream>
#include <algorithm>



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

	Ref<AssetManager> AssetManager::Create(const AssetManagerConfig& p_Config)
	{
		KTN_PROFILE_FUNCTION();

		auto assetManager = CreateRef<AssetManager>();
		s_Instance = assetManager;
		assetManager->m_Config = p_Config;

		return assetManager;
	}

	AssetManager::~AssetManager()
	{
		for (auto& [handle, metadata] : m_AssetRegistry)
		{
			if (metadata.AssetData != nullptr)
			{
				delete metadata.AssetData;
				metadata.AssetData = nullptr;
			}
		}
		m_AssetRegistry.clear();
	}

	AssetHandle AssetManager::ImportAsset(AssetType p_Type, const std::string& p_FilePath)
	{
		KTN_PROFILE_FUNCTION();

		if (auto handle = GetHandleByPath(p_FilePath); IsAssetHandleValid(handle))
		{
			return handle;
		}

		AssetMetadata metadata;
		metadata.FilePath = (Project::GetAssetDirectory() / FileSystem::GetRelative(p_FilePath, Project::GetAssetDirectory().string())).string();
		metadata.Type = p_Type;
		return ImportAsset(metadata);
	}

	AssetHandle AssetManager::ImportAsset(const AssetMetadata& p_Metadata)
	{
		if (auto handle = GetHandleByPath(p_Metadata.FilePath); IsAssetHandleValid(handle))
		{
			return handle;
		}

		AssetHandle handle; // generate new handle
		Ref<Asset> asset = AssetImporter::ImportAsset(handle, p_Metadata);
		if (asset)
		{
			asset->Handle = handle;
			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = p_Metadata;
			SerializeAssetRegistry();
			return handle;
		}

		KTN_CORE_ERROR("Failed to import asset: {}, {}", GetAssetTypeName(p_Metadata.Type), p_Metadata.FilePath);
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
				out << YAML::Key << "FilePath" << YAML::Value << FileSystem::GetRelative(metadata.FilePath, Project::GetAssetDirectory().string());
				out << YAML::Key << "Type" << YAML::Value << (std::string)GetAssetTypeName(metadata.Type);
				if (metadata.AssetData)
				{
					out << YAML::Key << "AssetData" << YAML::Value << YAML::BeginMap;
					auto assetData = metadata.AssetData;
					if (metadata.Type == AssetType::Font)
					{
						auto config = static_cast<MSDFFontConfig*>(assetData);
						out << YAML::Key << "ImageType" << YAML::Value << (std::string)magic_enum::enum_name(config->ImageType).data();
						out << YAML::Key << "GlyphIdentifier" << YAML::Value << (std::string)magic_enum::enum_name(config->GlyphIdentifier).data();
						out << YAML::Key << "ImageFormat" << YAML::Value << (std::string)magic_enum::enum_name(config->ImageFormat).data();
						out << YAML::Key << "EmSize" << YAML::Value << config->EmSize;
						out << YAML::Key << "PxRange" << YAML::Value << config->PxRange;
						out << YAML::Key << "MiterLimit" << YAML::Value << config->MiterLimit;
						out << YAML::Key << "AngleThreshold" << YAML::Value << config->AngleThreshold;
						out << YAML::Key << "FontScale" << YAML::Value << config->FontScale;
						out << YAML::Key << "ThreadCount" << YAML::Value << config->ThreadCount;
						out << YAML::Key << "ExpensiveColoring" << YAML::Value << config->ExpensiveColoring;
						out << YAML::Key << "FixedScale" << YAML::Value << config->FixedScale;
						out << YAML::Key << "OverlapSupport" << YAML::Value << config->OverlapSupport;
						out << YAML::Key << "ScanlinePass" << YAML::Value << config->ScanlinePass;
						out << YAML::Key << "UseDefaultCharset" << YAML::Value << config->UseDefaultCharset;
						out << YAML::Key << "CharsetRanges" << YAML::Value;
						out << YAML::BeginSeq;
						for (const auto& range : config->CharsetRanges)
						{
							out << YAML::BeginMap;
							out << YAML::Key << "Start" << YAML::Value << range.first;
							out << YAML::Key << "End" << YAML::Value << range.second;
							out << YAML::EndMap;
						}
						out << YAML::EndSeq;
					}

					if (metadata.Type == AssetType::Texture2D)
					{
						auto spec = static_cast<TextureSpecification*>(assetData);
						out << YAML::Key << "MinFilter" << YAML::Value << (std::string)magic_enum::enum_name(spec->MinFilter);
						out << YAML::Key << "MagFilter" << YAML::Value << (std::string)magic_enum::enum_name(spec->MagFilter);
						out << YAML::Key << "WrapU" << YAML::Value << (std::string)magic_enum::enum_name(spec->WrapU);
						out << YAML::Key << "WrapV" << YAML::Value << (std::string)magic_enum::enum_name(spec->WrapV);				
						out << YAML::Key << "SRGB" << YAML::Value << spec->SRGB;
						out << YAML::Key << "AnisotropyEnable" << YAML::Value << spec->AnisotropyEnable;
						out << YAML::Key << "GenerateMipmaps" << YAML::Value << spec->GenerateMips;
						out << YAML::Key << "BorderColor" << YAML::Value << spec->BorderColor;
						out << YAML::Key << "DebugName" << YAML::Value << spec->DebugName;
					}

					out << YAML::EndMap;
				}
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

			if (node["AssetData"])
			{
				auto assetDataNode = node["AssetData"];
				if (metadata.Type == AssetType::Font)
				{
					auto config = new MSDFFontConfig();

					config->ImageType = magic_enum::enum_cast<FontImageType>(assetDataNode["ImageType"].as<std::string>().c_str()).value_or(config->ImageType);
					config->GlyphIdentifier = magic_enum::enum_cast<GlyphIdentifierType>(assetDataNode["GlyphIdentifier"].as<std::string>().c_str()).value_or(config->GlyphIdentifier);
					config->ImageFormat = magic_enum::enum_cast<FontImageFormat>(assetDataNode["ImageFormat"].as<std::string>().c_str()).value_or(config->ImageFormat);
					config->EmSize = assetDataNode["EmSize"].as<float>();
					config->PxRange = assetDataNode["PxRange"].as<double>();
					config->MiterLimit = assetDataNode["MiterLimit"].as<double>();
					config->AngleThreshold = assetDataNode["AngleThreshold"].as<double>();
					config->FontScale = assetDataNode["FontScale"].as<double>();
					config->ThreadCount = assetDataNode["ThreadCount"].as<int>();
					config->ExpensiveColoring = assetDataNode["ExpensiveColoring"].as<bool>();
					config->FixedScale = assetDataNode["FixedScale"].as<bool>();
					config->OverlapSupport = assetDataNode["OverlapSupport"].as<bool>();
					config->ScanlinePass = assetDataNode["ScanlinePass"].as<bool>();
					config->UseDefaultCharset = assetDataNode["UseDefaultCharset"].as<bool>();
					config->CharsetRanges.clear();
					for (const auto& range : assetDataNode["CharsetRanges"])
					{
						uint32_t start = range["Start"].as<uint32_t>();
						uint32_t end = range["End"].as<uint32_t>();
						config->CharsetRanges.push_back({ start, end });
					}
					metadata.AssetData = config;
				}

				if (metadata.Type == AssetType::Texture2D)
				{
					auto spec = new TextureSpecification();
					spec->MinFilter = magic_enum::enum_cast<TextureFilter>(assetDataNode["MinFilter"].as<std::string>().c_str()).value_or(TextureFilter::LINEAR);
					spec->MagFilter = magic_enum::enum_cast<TextureFilter>(assetDataNode["MagFilter"].as<std::string>().c_str()).value_or(TextureFilter::LINEAR);
					spec->WrapU = magic_enum::enum_cast<TextureWrap>(assetDataNode["WrapU"].as<std::string>().c_str()).value_or(TextureWrap::REPEAT);
					spec->WrapV = magic_enum::enum_cast<TextureWrap>(assetDataNode["WrapV"].as<std::string>().c_str()).value_or(TextureWrap::REPEAT);
					spec->SRGB = assetDataNode["SRGB"].as<bool>();
					spec->AnisotropyEnable = assetDataNode["AnisotropyEnable"].as<bool>();
					spec->GenerateMips = assetDataNode["GenerateMipmaps"].as<bool>();
					spec->BorderColor = assetDataNode["BorderColor"].as<glm::vec4>();
					spec->DebugName = assetDataNode["DebugName"].as<std::string>();
					metadata.AssetData = spec;
				}
			}
		}

		return true;
	}


} // namespace KTN
