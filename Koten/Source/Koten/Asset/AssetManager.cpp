#include "ktnpch.h"
#include "AssetManager.h"
#include "AssetImporter.h"
#include "Koten/Project/Project.h"
#include "Koten/Graphics/DFFont.h"
#include "Koten/Scene/Entity.h"
#include "Koten/Script/ScriptEngine.h"
#include "Koten/Utils/Utils.h"
#include "Koten/Scene/SceneSerializer.h"
#include "Koten/Physics/PhysicsMaterial2D.h"

// lib
#include <yaml-cpp/yaml.h>
#include <magic_enum/magic_enum.hpp>

// std
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
		});

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
		KTN_PROFILE_FUNCTION();

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

	AssetHandle AssetManager::ImportAsset(AssetType p_Type, const std::string& p_FilePath, bool p_Force)
	{
		KTN_PROFILE_FUNCTION();

		AssetMetadata metadata;
		metadata.FilePath = (Project::GetAssetDirectory() / FileSystem::GetRelative(p_FilePath, Project::GetAssetDirectory().string())).string();
		metadata.Type = p_Type;
		return ImportAsset(metadata, p_Force);
	}

	AssetHandle AssetManager::ImportAsset(const AssetMetadata& p_Metadata, bool p_Force)
	{
		KTN_PROFILE_FUNCTION();

		if (auto handle = GetHandleByPath(p_Metadata.FilePath);
			IsAssetHandleValid(handle) && !p_Force)
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
			if (!m_IsLoadedAssetPack)
				SerializeAssetRegistry();
			else
				m_NeedsToUpdate = true;
			return handle;
		}

		KTN_CORE_ERROR("Failed to import asset: {}, {}", GetAssetTypeName(p_Metadata.Type), p_Metadata.FilePath);
		return 0;
	}

	bool AssetManager::ImportAsset(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Ref<Asset>& p_Asset)
	{
		KTN_PROFILE_FUNCTION();

		Ref<Asset> asset = p_Asset;
		AssetHandle handle = p_Handle != 0 ? p_Handle : AssetHandle();
		if (!asset)
			asset = AssetImporter::ImportAsset(handle, p_Metadata);

		if (asset)
		{
			asset->Handle = handle;
			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = p_Metadata;
			if (!m_IsLoadedAssetPack)
				SerializeAssetRegistry();
			else
				m_NeedsToUpdate = true;
			return true;
		}

		KTN_CORE_ERROR("Failed to import asset: {}, {}", GetAssetTypeName(p_Metadata.Type), p_Metadata.FilePath);
		return false;
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

	bool AssetManager::HasAsset(AssetType p_Type, const std::string& p_FilePath) const
	{
		KTN_PROFILE_FUNCTION();

		auto it = std::find_if(m_AssetRegistry.begin(), m_AssetRegistry.end(),
		[&p_Type, &p_FilePath](const auto& p_Pair)
		{
			return p_Pair.second.Type == p_Type && p_Pair.second.FilePath == p_FilePath;
		});

		return it != m_AssetRegistry.end();
	}

	bool AssetManager::RemoveAsset(AssetHandle p_Handle)
	{
		KTN_PROFILE_FUNCTION();

		auto it = m_AssetRegistry.find(p_Handle);
		if (it != m_AssetRegistry.end())
		{
			if (m_LoadedAssets.find(p_Handle) != m_LoadedAssets.end())
				m_LoadedAssets.erase(p_Handle);

			m_AssetRegistry.erase(p_Handle);

			if (!m_IsLoadedAssetPack)
				SerializeAssetRegistry();
			else
				SerializeAssetPack();
			return true;
		}

		KTN_CORE_ERROR("This asset not exists");
		return false;
	}

	const AssetMetadata& AssetManager::GetMetadata(AssetHandle p_Handle) const
	{
		KTN_PROFILE_FUNCTION();

		static AssetMetadata s_EmptyMetadata;
		if (!IsAssetHandleValid(p_Handle))
			return s_EmptyMetadata;

		return m_AssetRegistry.at(p_Handle);
	}

	AssetMetadata& AssetManager::GetMetadata(AssetHandle p_Handle)
	{
		KTN_PROFILE_FUNCTION();

		static AssetMetadata s_EmptyMetadata;
		if (!IsAssetHandleValid(p_Handle))
			return s_EmptyMetadata;

		return m_AssetRegistry.at(p_Handle);
	}

	struct AssetPackHeader
	{
		char Magic[4] = { 'K', 'T', 'A', 'P' };
		uint32_t Version = 2;
		size_t AssetRegistrySize = 0;
	};

	static void WriteTextureSpecification(std::ofstream& p_Stream, const TextureSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION();

		p_Stream.write(reinterpret_cast<const char*>(&p_Spec.Width), sizeof(p_Spec.Width));
		p_Stream.write(reinterpret_cast<const char*>(&p_Spec.Height), sizeof(p_Spec.Height));
		p_Stream.write(reinterpret_cast<const char*>(&p_Spec.MinFilter), sizeof(p_Spec.MinFilter));
		p_Stream.write(reinterpret_cast<const char*>(&p_Spec.MagFilter), sizeof(p_Spec.MagFilter));
		p_Stream.write(reinterpret_cast<const char*>(&p_Spec.WrapU), sizeof(p_Spec.WrapU));
		p_Stream.write(reinterpret_cast<const char*>(&p_Spec.WrapV), sizeof(p_Spec.WrapV));
		p_Stream.write(reinterpret_cast<const char*>(&p_Spec.SRGB), sizeof(p_Spec.SRGB));
		p_Stream.write(reinterpret_cast<const char*>(&p_Spec.AnisotropyEnable), sizeof(p_Spec.AnisotropyEnable));
		p_Stream.write(reinterpret_cast<const char*>(&p_Spec.GenerateMips), sizeof(p_Spec.GenerateMips));
		p_Stream.write(reinterpret_cast<const char*>(&p_Spec.BorderColor), sizeof(p_Spec.BorderColor));

		Utils::WriteString(p_Stream, p_Spec.DebugName);
	}

	static void ReadTextureSpecification(std::ifstream& p_File, TextureSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION();

		p_File.read(reinterpret_cast<char*>(&p_Spec.Width), sizeof(p_Spec.Width));
		p_File.read(reinterpret_cast<char*>(&p_Spec.Height), sizeof(p_Spec.Height));
		p_File.read(reinterpret_cast<char*>(&p_Spec.MinFilter), sizeof(p_Spec.MinFilter));
		p_File.read(reinterpret_cast<char*>(&p_Spec.MagFilter), sizeof(p_Spec.MagFilter));
		p_File.read(reinterpret_cast<char*>(&p_Spec.WrapU), sizeof(p_Spec.WrapU));
		p_File.read(reinterpret_cast<char*>(&p_Spec.WrapV), sizeof(p_Spec.WrapV));
		p_File.read(reinterpret_cast<char*>(&p_Spec.SRGB), sizeof(p_Spec.SRGB));
		p_File.read(reinterpret_cast<char*>(&p_Spec.AnisotropyEnable), sizeof(p_Spec.AnisotropyEnable));
		p_File.read(reinterpret_cast<char*>(&p_Spec.GenerateMips), sizeof(p_Spec.GenerateMips));
		p_File.read(reinterpret_cast<char*>(&p_Spec.BorderColor), sizeof(p_Spec.BorderColor));

		p_Spec.DebugName = Utils::ReadString(p_File);
	}

	void AssetManager::SerializeAssetPack(const std::filesystem::path& p_Folder)
	{
		KTN_PROFILE_FUNCTION();

		FileSystem::CreateDirectories(p_Folder.string());
		const auto cachePath = p_Folder / "AssetPack.ktap";

		KTN_CORE_INFO("Serializing AssetPack to {}...", cachePath.string());

		std::ofstream out(cachePath, std::ios::binary | std::ios::trunc);
		if (!out)
		{
			KTN_CORE_ERROR("Failed to create asset pack at {}", cachePath.string());
			return;
		}

		AssetPackHeader header;
		header.AssetRegistrySize = m_AssetRegistry.size();

		out.write(reinterpret_cast<const char*>(&header), sizeof(header));

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			out.write(reinterpret_cast<const char*>(&handle), sizeof(handle));
			out.write(reinterpret_cast<const char*>(&metadata.Type), sizeof(metadata.Type));
			Utils::WriteString(out, FileSystem::GetRelative(metadata.FilePath, Project::GetAssetDirectory().string()));

			bool hasAssetData = (metadata.AssetData != nullptr);
			out.write(reinterpret_cast<const char*>(&hasAssetData), sizeof(hasAssetData));

			if (hasAssetData)
			{
				switch (metadata.Type)
				{
					case AssetType::Font:
					{
						auto config = static_cast<const DFFontConfig*>(metadata.AssetData);

						out.write(reinterpret_cast<const char*>(&config->ImageType), sizeof(config->ImageType));
						out.write(reinterpret_cast<const char*>(&config->GlyphIdentifier), sizeof(config->GlyphIdentifier));
						out.write(reinterpret_cast<const char*>(&config->ImageFormat), sizeof(config->ImageFormat));
						out.write(reinterpret_cast<const char*>(&config->EmSize), sizeof(config->EmSize));
						out.write(reinterpret_cast<const char*>(&config->PxRange), sizeof(config->PxRange));
						out.write(reinterpret_cast<const char*>(&config->MiterLimit), sizeof(config->MiterLimit));
						out.write(reinterpret_cast<const char*>(&config->AngleThreshold), sizeof(config->AngleThreshold));
						out.write(reinterpret_cast<const char*>(&config->FontScale), sizeof(config->FontScale));
						out.write(reinterpret_cast<const char*>(&config->ThreadCount), sizeof(config->ThreadCount));
						out.write(reinterpret_cast<const char*>(&config->ExpensiveColoring), sizeof(config->ExpensiveColoring));
						out.write(reinterpret_cast<const char*>(&config->FixedScale), sizeof(config->FixedScale));
						out.write(reinterpret_cast<const char*>(&config->OverlapSupport), sizeof(config->OverlapSupport));
						out.write(reinterpret_cast<const char*>(&config->ScanlinePass), sizeof(config->ScanlinePass));
						out.write(reinterpret_cast<const char*>(&config->UseDefaultCharset), sizeof(config->UseDefaultCharset));

						if (!config->UseDefaultCharset)
						{
							size_t numRanges = config->CharsetRanges.size();
							out.write(reinterpret_cast<const char*>(&numRanges), sizeof(numRanges));

							for (const auto& range : config->CharsetRanges)
							{
								out.write(reinterpret_cast<const char*>(&range.first), sizeof(uint32_t));
								out.write(reinterpret_cast<const char*>(&range.second), sizeof(uint32_t));
							}
						}
						break;
					}
					case AssetType::Texture2D:
					{
						auto spec = static_cast<const TextureSpecification*>(metadata.AssetData);
						WriteTextureSpecification(out, *spec);
						break;
					}
				}
			}

			if (metadata.Type == AssetType::Texture2D)
			{
				auto texture = As<Asset, Texture2D>(GetAsset(handle));
				if (texture)
				{
					const auto& spec = texture->GetSpecification();
					WriteTextureSpecification(out, spec);

					size_t dataSize = texture->GetEstimatedSize();
					out.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

					std::vector<uint8_t> textureData = texture->GetData();
					out.write(reinterpret_cast<const char*>(textureData.data()), dataSize);
				}
			}

			if (metadata.Type == AssetType::Scene)
			{
				auto scene = As<Asset, Scene>(GetAsset(handle));
				KTN_CORE_ASSERT(scene, "Scene is nullptr!");

				SceneSerializer serializer(scene);
				serializer.SerializeBin(out);
			}

			if (metadata.Type == AssetType::PhysicsMaterial2D)
			{
				auto material = As<Asset, PhysicsMaterial2D>(GetAsset(handle));
				KTN_CORE_ASSERT(material, "PhysicsMaterial2D is nullptr!");

				material->SerializeBin(out);
			}
		}
	}

	bool AssetManager::DeserializeAssetPack(const std::filesystem::path& p_Folder)
	{
		KTN_PROFILE_FUNCTION();

		const auto cachePath = p_Folder / "AssetPack.ktap";
		m_IsLoadedAssetPack = true;

		std::ifstream in(cachePath, std::ios::binary);
		if (!in)
		{
			KTN_CORE_ERROR("Failed to open asset pack file: {}", cachePath.string());
			return false;
		}

		AssetPackHeader header;
		in.read(reinterpret_cast<char*>(&header), sizeof(header));
		if (memcmp(header.Magic, "KTAP", 4) != 0 || header.Version != 2)
		{
			KTN_CORE_ERROR("Invalid asset pack format");
			return false;
		}

		for (size_t i = 0; i < header.AssetRegistrySize; ++i)
		{
			AssetHandle handle;
			AssetMetadata metadata;

			in.read(reinterpret_cast<char*>(&handle), sizeof(handle));
			in.read(reinterpret_cast<char*>(&metadata.Type), sizeof(metadata.Type));
			metadata.FilePath = (Project::GetAssetDirectory() / Utils::ReadString(in)).string();

			bool hasAssetData = false;
			in.read(reinterpret_cast<char*>(&hasAssetData), sizeof(hasAssetData));
			if (hasAssetData)
			{
				switch (metadata.Type)
				{
					case AssetType::Font:
					{
						auto config = new DFFontConfig();

						in.read(reinterpret_cast<char*>(&config->ImageType), sizeof(config->ImageType));
						in.read(reinterpret_cast<char*>(&config->GlyphIdentifier), sizeof(config->GlyphIdentifier));
						in.read(reinterpret_cast<char*>(&config->ImageFormat), sizeof(config->ImageFormat));
						in.read(reinterpret_cast<char*>(&config->EmSize), sizeof(config->EmSize));
						in.read(reinterpret_cast<char*>(&config->PxRange), sizeof(config->PxRange));
						in.read(reinterpret_cast<char*>(&config->MiterLimit), sizeof(config->MiterLimit));
						in.read(reinterpret_cast<char*>(&config->AngleThreshold), sizeof(config->AngleThreshold));
						in.read(reinterpret_cast<char*>(&config->FontScale), sizeof(config->FontScale));
						in.read(reinterpret_cast<char*>(&config->ThreadCount), sizeof(config->ThreadCount));
						in.read(reinterpret_cast<char*>(&config->ExpensiveColoring), sizeof(config->ExpensiveColoring));
						in.read(reinterpret_cast<char*>(&config->FixedScale), sizeof(config->FixedScale));
						in.read(reinterpret_cast<char*>(&config->OverlapSupport), sizeof(config->OverlapSupport));
						in.read(reinterpret_cast<char*>(&config->ScanlinePass), sizeof(config->ScanlinePass));
						in.read(reinterpret_cast<char*>(&config->UseDefaultCharset), sizeof(config->UseDefaultCharset));

						if (!config->UseDefaultCharset)
						{
							size_t numRanges = 0;
							in.read(reinterpret_cast<char*>(&numRanges), sizeof(numRanges));

							config->CharsetRanges.clear();
							config->CharsetRanges.resize(numRanges);
							for (size_t i = 0; i < numRanges; ++i)
							{
								uint32_t start{ 0 }, end{ 0 };
								in.read(reinterpret_cast<char*>(&start), sizeof(uint32_t));
								in.read(reinterpret_cast<char*>(&end), sizeof(uint32_t));
								config->CharsetRanges[i] = { start, end };
							}
						}

						metadata.AssetData = config;
						break;
					}
					case AssetType::Texture2D:
					{
						auto spec = new TextureSpecification();
						ReadTextureSpecification(in, *spec);
						metadata.AssetData = spec;
						break;
					}
				}
			}

			if (metadata.Type == AssetType::Texture2D)
			{
				TextureSpecification spec = {};
				ReadTextureSpecification(in, spec);

				size_t dataSize = 0;
				in.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

				std::vector<uint8_t> textureData(dataSize);
				in.read(reinterpret_cast<char*>(textureData.data()), dataSize);

				auto texture = Texture2D::Create(spec);
				texture->SetData(textureData.data(), dataSize);
				m_LoadedAssets[handle] = texture;
			}

			if (metadata.Type == AssetType::Scene)
			{
				auto scene = CreateRef<Scene>();
				scene->Handle = handle;

				SceneSerializer serializer(scene);
				serializer.DeserializeBin(in);

				m_LoadedAssets[handle] = scene;
			}

			if (metadata.Type == AssetType::PhysicsMaterial2D)
			{
				auto material = CreateRef<PhysicsMaterial2D>();
				material->Handle = handle;

				material->DeserializeBin(in);

				m_LoadedAssets[handle] = material;
			}

			m_AssetRegistry[handle] = metadata;
		}

		KTN_CORE_INFO("Deserialized {} assets from pack", m_AssetRegistry.size());

		if (m_NeedsToUpdate)
		{
			SerializeAssetPack(p_Folder);
			m_NeedsToUpdate = false;
		}

		return true;
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
						auto config = static_cast<DFFontConfig*>(assetData);
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
						if (!config->UseDefaultCharset)
						{
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
					auto config = new DFFontConfig();

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
					if (!config->UseDefaultCharset)
					{
						config->CharsetRanges.clear();
						for (const auto& range : assetDataNode["CharsetRanges"])
						{
							uint32_t start = range["Start"].as<uint32_t>();
							uint32_t end = range["End"].as<uint32_t>();
							config->CharsetRanges.push_back({ start, end });
						}
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
