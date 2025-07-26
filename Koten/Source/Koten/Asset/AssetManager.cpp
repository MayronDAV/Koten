#include "ktnpch.h"
#include "AssetManager.h"
#include "AssetImporter.h"
#include "Koten/Project/Project.h"
#include "Koten/Graphics/DFFont.h"
#include "Koten/Scene/Entity.h"
#include "Koten/Script/ScriptEngine.h"
#include "Koten/Utils/Utils.h"

// lib
#include <yaml-cpp/yaml.h>
#include <magic_enum/magic_enum.hpp>
#include <entt/entt.hpp>

// std
#include <algorithm>



namespace KTN
{
	namespace
	{
		template<typename Component>
		void ComponentSerializeBinIfExist(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity) {}

		template<typename Component>
		void ComponentDeserializeBinIfExist(std::ifstream& p_Input, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity) {}

		template<typename... Component>
		void ComponentSerializeBin(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			int count = 0;
			auto countComponents = [&count, &p_Entity]<typename T>() 
			{
				if (p_Entity.HasComponent<T>())
				{
					count++;
				}
			};
			(countComponents.template operator()<Component>(), ...);

			count = count > 0 ? count - 1 : 0; // Exclude the IDComponent
			p_Out.write(reinterpret_cast<const char*>(&count), sizeof(count));
			(ComponentSerializeBinIfExist<Component>(p_Out, p_Registry, p_Entity), ...);
		}

		template<typename... Component>
		void ComponentDeserializeBin(std::ifstream& p_Input, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			int count = 0;
			p_Input.read(reinterpret_cast<char*>(&count), sizeof(count));
			if (count <= 0)
			{
				KTN_CORE_WARN("ComponentDeserializeBin - No components to deserialize! Entity: ( Tag: {}, ID: {} )", p_Entity.GetTag(), (uint64_t)p_Entity.GetUUID());
				return;
			}

			for (int i = 0; i < count; i++)
			{
				auto current = Utils::ReadString(p_Input);
				(ComponentDeserializeBinIfExist<Component>(p_Input, current, p_Registry, p_Entity), ...);
			}
		}

	} // namespace

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
		auto it = std::find_if(m_AssetRegistry.begin(), m_AssetRegistry.end(),
		[&p_Type, &p_FilePath](const auto& p_Pair)
		{
			return p_Pair.second.Type == p_Type && p_Pair.second.FilePath == p_FilePath;
		});

		return it != m_AssetRegistry.end();
	}

	const AssetMetadata& AssetManager::GetMetadata(AssetHandle p_Handle) const
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
		uint32_t Version = 1;
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

		std::ofstream stream(cachePath, std::ios::binary | std::ios::trunc);
		if (!stream)
		{
			KTN_CORE_ERROR("Failed to create asset pack at {}", cachePath.string());
			return;
		}

		AssetPackHeader header;
		header.AssetRegistrySize = m_AssetRegistry.size();

		stream.write(reinterpret_cast<const char*>(&header), sizeof(header));

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			stream.write(reinterpret_cast<const char*>(&handle), sizeof(handle));
			stream.write(reinterpret_cast<const char*>(&metadata.Type), sizeof(metadata.Type));
			Utils::WriteString(stream, metadata.FilePath);

			bool hasAssetData = (metadata.AssetData != nullptr);
			stream.write(reinterpret_cast<const char*>(&hasAssetData), sizeof(hasAssetData));

			if (hasAssetData)
			{
				switch (metadata.Type)
				{
				case AssetType::Font:
				{
					auto config = static_cast<const DFFontConfig*>(metadata.AssetData);

					stream.write(reinterpret_cast<const char*>(&config->ImageType), sizeof(config->ImageType));
					stream.write(reinterpret_cast<const char*>(&config->GlyphIdentifier), sizeof(config->GlyphIdentifier));
					stream.write(reinterpret_cast<const char*>(&config->ImageFormat), sizeof(config->ImageFormat));
					stream.write(reinterpret_cast<const char*>(&config->EmSize), sizeof(config->EmSize));
					stream.write(reinterpret_cast<const char*>(&config->PxRange), sizeof(config->PxRange));
					stream.write(reinterpret_cast<const char*>(&config->MiterLimit), sizeof(config->MiterLimit));
					stream.write(reinterpret_cast<const char*>(&config->AngleThreshold), sizeof(config->AngleThreshold));
					stream.write(reinterpret_cast<const char*>(&config->FontScale), sizeof(config->FontScale));
					stream.write(reinterpret_cast<const char*>(&config->ThreadCount), sizeof(config->ThreadCount));
					stream.write(reinterpret_cast<const char*>(&config->ExpensiveColoring), sizeof(config->ExpensiveColoring));
					stream.write(reinterpret_cast<const char*>(&config->FixedScale), sizeof(config->FixedScale));
					stream.write(reinterpret_cast<const char*>(&config->OverlapSupport), sizeof(config->OverlapSupport));
					stream.write(reinterpret_cast<const char*>(&config->ScanlinePass), sizeof(config->ScanlinePass));
					stream.write(reinterpret_cast<const char*>(&config->UseDefaultCharset), sizeof(config->UseDefaultCharset));

					if (!config->UseDefaultCharset)
					{
						size_t numRanges = config->CharsetRanges.size();
						stream.write(reinterpret_cast<const char*>(&numRanges), sizeof(numRanges));

						for (const auto& range : config->CharsetRanges)
						{
							stream.write(reinterpret_cast<const char*>(&range.first), sizeof(uint32_t));
							stream.write(reinterpret_cast<const char*>(&range.second), sizeof(uint32_t));
						}
					}
					break;
				}
				case AssetType::Texture2D:
				{
					auto spec = static_cast<const TextureSpecification*>(metadata.AssetData);
					WriteTextureSpecification(stream, *spec);
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
					WriteTextureSpecification(stream, spec);

					size_t dataSize = texture->GetEstimatedSize();
					stream.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

					std::vector<uint8_t> textureData = texture->GetData();
					stream.write(reinterpret_cast<const char*>(textureData.data()), dataSize);
				}
			}

			if (metadata.Type == AssetType::Scene)
			{
				auto scene = As<Asset, Scene>(GetAsset(handle));
				KTN_CORE_ASSERT(scene, "Scene is nullptr!");

				auto entts = scene->GetRegistry().view<entt::entity>();

				int size = 0;
				for (auto entity : entts)
					size++;
				stream.write(reinterpret_cast<const char*>(&size), sizeof(size));

				for (auto entity : entts)
				{
					auto entt = Entity{ entity, scene.get() };

					auto uuid = entt.GetUUID();
					stream.write(reinterpret_cast<const char*>(&uuid), sizeof(uuid));

					ComponentSerializeBin<ALL_COMPONENTS>(stream, scene->GetRegistry(), entt);
				}
			}
		}
	}

	bool AssetManager::DeserializeAssetPack(const std::filesystem::path& p_Folder)
	{
		KTN_PROFILE_FUNCTION();

		const auto cachePath = p_Folder / "AssetPack.ktap";
		m_IsLoadedAssetPack = true;

		std::ifstream file(cachePath, std::ios::binary);
		if (!file)
		{
			KTN_CORE_ERROR("Failed to open asset pack file: {}", cachePath.string());
			return false;
		}

		AssetPackHeader header;
		file.read(reinterpret_cast<char*>(&header), sizeof(header));
		if (memcmp(header.Magic, "KTAP", 4) != 0 || header.Version != 1)
		{
			KTN_CORE_ERROR("Invalid asset pack format");
			return false;
		}

		for (size_t i = 0; i < header.AssetRegistrySize; ++i)
		{
			AssetHandle handle;
			AssetMetadata metadata;

			file.read(reinterpret_cast<char*>(&handle), sizeof(handle));
			file.read(reinterpret_cast<char*>(&metadata.Type), sizeof(metadata.Type));
			metadata.FilePath = Utils::ReadString(file);

			bool hasAssetData = false;
			file.read(reinterpret_cast<char*>(&hasAssetData), sizeof(hasAssetData));
			if (hasAssetData)
			{
				switch (metadata.Type)
				{
					case AssetType::Font:
					{
						auto config = new DFFontConfig();

						file.read(reinterpret_cast<char*>(&config->ImageType), sizeof(config->ImageType));
						file.read(reinterpret_cast<char*>(&config->GlyphIdentifier), sizeof(config->GlyphIdentifier));
						file.read(reinterpret_cast<char*>(&config->ImageFormat), sizeof(config->ImageFormat));
						file.read(reinterpret_cast<char*>(&config->EmSize), sizeof(config->EmSize));
						file.read(reinterpret_cast<char*>(&config->PxRange), sizeof(config->PxRange));
						file.read(reinterpret_cast<char*>(&config->MiterLimit), sizeof(config->MiterLimit));
						file.read(reinterpret_cast<char*>(&config->AngleThreshold), sizeof(config->AngleThreshold));
						file.read(reinterpret_cast<char*>(&config->FontScale), sizeof(config->FontScale));
						file.read(reinterpret_cast<char*>(&config->ThreadCount), sizeof(config->ThreadCount));
						file.read(reinterpret_cast<char*>(&config->ExpensiveColoring), sizeof(config->ExpensiveColoring));
						file.read(reinterpret_cast<char*>(&config->FixedScale), sizeof(config->FixedScale));
						file.read(reinterpret_cast<char*>(&config->OverlapSupport), sizeof(config->OverlapSupport));
						file.read(reinterpret_cast<char*>(&config->ScanlinePass), sizeof(config->ScanlinePass));
						file.read(reinterpret_cast<char*>(&config->UseDefaultCharset), sizeof(config->UseDefaultCharset));

						if (!config->UseDefaultCharset)
						{
							size_t numRanges = 0;
							file.read(reinterpret_cast<char*>(&numRanges), sizeof(numRanges));

							config->CharsetRanges.clear();
							config->CharsetRanges.resize(numRanges);
							for (size_t i = 0; i < numRanges; ++i)
							{
								uint32_t start{ 0 }, end{ 0 };
								file.read(reinterpret_cast<char*>(&start), sizeof(uint32_t));
								file.read(reinterpret_cast<char*>(&end), sizeof(uint32_t));
								config->CharsetRanges[i] = { start, end };
							}
						}

						metadata.AssetData = config;
						break;
					}
					case AssetType::Texture2D:
					{
						auto spec = new TextureSpecification();
						ReadTextureSpecification(file, *spec);
						metadata.AssetData = spec;
						break;
					}
				}
			}

			if (metadata.Type == AssetType::Texture2D)
			{
				TextureSpecification spec = {};
				ReadTextureSpecification(file, spec);

				size_t dataSize = 0;
				file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

				std::vector<uint8_t> textureData(dataSize);
				file.read(reinterpret_cast<char*>(textureData.data()), dataSize);

				auto texture = Texture2D::Create(spec);
				texture->SetData(textureData.data(), dataSize);
				m_LoadedAssets[handle] = texture;
			}

			if (metadata.Type == AssetType::Scene)
			{
				auto scene = CreateRef<Scene>();
				scene->Handle = handle;

				int size = 0;
				file.read(reinterpret_cast<char*>(&size), sizeof(size));

				for (int i = 0; i < size; i++)
				{
					UUID uuid;
					file.read(reinterpret_cast<char*>(&uuid), sizeof(uuid));

					Entity entt = scene->CreateEntity(uuid);

					ComponentDeserializeBin<ALL_COMPONENTS>(file, scene->GetRegistry(), entt);
				}

				m_LoadedAssets[handle] = scene;
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

	// Serialize
	namespace
	{
		template<>
		void ComponentSerializeBinIfExist<TagComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<TagComponent>())
				return;

			Utils::WriteString(p_Out, "TagComponent");

			Utils::WriteString(p_Out, p_Entity.GetTag());
		}

		template<>
		void ComponentSerializeBinIfExist<HierarchyComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<HierarchyComponent>())
				return;

			Utils::WriteString(p_Out, "HierarchyComponent");

			auto& comp = p_Entity.GetComponent<HierarchyComponent>();

			UUID null = 0ul;

			if (comp.Parent != entt::null)
			{
				auto entt = Entity{ comp.Parent, p_Entity.GetScene() };
				auto uuid = entt.GetUUID();
				p_Out.write(reinterpret_cast<const char*>(&uuid), sizeof(UUID));
			}
			else
				p_Out.write(reinterpret_cast<const char*>(&null), sizeof(UUID));

			if (comp.First != entt::null)
			{
				auto entt = Entity{ comp.First, p_Entity.GetScene() };
				auto uuid = entt.GetUUID();
				p_Out.write(reinterpret_cast<const char*>(&uuid), sizeof(UUID));
			}
			else
				p_Out.write(reinterpret_cast<const char*>(&null), sizeof(UUID));

			if (comp.Prev != entt::null)
			{
				auto entt = Entity{ comp.Prev, p_Entity.GetScene() };
				auto uuid = entt.GetUUID();
				p_Out.write(reinterpret_cast<const char*>(&uuid), sizeof(UUID));
			}
			else
				p_Out.write(reinterpret_cast<const char*>(&null), sizeof(UUID));

			if (comp.Next != entt::null)
			{
				auto entt = Entity{ comp.Next, p_Entity.GetScene() };
				auto uuid = entt.GetUUID();
				p_Out.write(reinterpret_cast<const char*>(&uuid), sizeof(UUID));
			}
			else
				p_Out.write(reinterpret_cast<const char*>(&null), sizeof(UUID));

			p_Out.write(reinterpret_cast<const char*>(&comp.ChildCount), sizeof(comp.ChildCount));
		}

		template<>
		void ComponentSerializeBinIfExist<TransformComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<TransformComponent>())
				return;

			Utils::WriteString(p_Out, "TransformComponent");

			auto& comp = p_Entity.GetComponent<TransformComponent>();

			glm::vec3 translation = comp.GetLocalTranslation();
			p_Out.write(reinterpret_cast<const char*>(&translation), sizeof(translation));
			glm::vec3 rotation = comp.GetLocalRotation();
			p_Out.write(reinterpret_cast<const char*>(&rotation), sizeof(rotation));
			glm::vec3 scale = comp.GetLocalScale();
			p_Out.write(reinterpret_cast<const char*>(&scale), sizeof(scale));
		}

		template<>
		void ComponentSerializeBinIfExist<CameraComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<CameraComponent>())
				return;

			Utils::WriteString(p_Out, "CameraComponent");

			auto& comp = p_Entity.GetComponent<CameraComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.Primary), sizeof(comp.Primary));
			p_Out.write(reinterpret_cast<const char*>(&comp.ClearColor), sizeof(comp.ClearColor));
			uint32_t width = comp.Camera.GetViewportWidth();
			p_Out.write(reinterpret_cast<const char*>(&width), sizeof(width));
			uint32_t height = comp.Camera.GetViewportHeight();
			p_Out.write(reinterpret_cast<const char*>(&height), sizeof(height));
			bool isOrthographic = comp.Camera.IsOrthographic();
			p_Out.write(reinterpret_cast<const char*>(&isOrthographic), sizeof(isOrthographic));
			bool isAspectRatioFixed = comp.Camera.IsAspectRatioFixed();
			p_Out.write(reinterpret_cast<const char*>(&isAspectRatioFixed), sizeof(isAspectRatioFixed));
			float fov = comp.Camera.GetFOV();
			p_Out.write(reinterpret_cast<const char*>(&fov), sizeof(fov));
			float farZ = comp.Camera.GetFar();
			p_Out.write(reinterpret_cast<const char*>(&farZ), sizeof(farZ));
			float nearZ = comp.Camera.GetNear();
			p_Out.write(reinterpret_cast<const char*>(&nearZ), sizeof(nearZ));
			float scale = comp.Camera.GetScale();
			p_Out.write(reinterpret_cast<const char*>(&scale), sizeof(scale));
			float zoom = comp.Camera.GetZoom();
			p_Out.write(reinterpret_cast<const char*>(&zoom), sizeof(zoom));
		}

		template<>
		void ComponentSerializeBinIfExist<SpriteComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<SpriteComponent>())
				return;

			Utils::WriteString(p_Out, "SpriteComponent");

			auto& comp = p_Entity.GetComponent<SpriteComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.Type), sizeof(comp.Type));
			p_Out.write(reinterpret_cast<const char*>(&comp.Color), sizeof(comp.Color));
			p_Out.write(reinterpret_cast<const char*>(&comp.Texture), sizeof(comp.Texture));
			p_Out.write(reinterpret_cast<const char*>(&comp.Thickness), sizeof(comp.Thickness));
			p_Out.write(reinterpret_cast<const char*>(&comp.Fade), sizeof(comp.Fade));
			p_Out.write(reinterpret_cast<const char*>(&comp.Size), sizeof(comp.Size));
			p_Out.write(reinterpret_cast<const char*>(&comp.BySize), sizeof(comp.BySize));
			p_Out.write(reinterpret_cast<const char*>(&comp.Offset), sizeof(comp.Offset));
			p_Out.write(reinterpret_cast<const char*>(&comp.Scale), sizeof(comp.Scale));
		}

		template<>
		void ComponentSerializeBinIfExist<LineRendererComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<LineRendererComponent>())
				return;

			Utils::WriteString(p_Out, "LineRendererComponent");

			auto& comp = p_Entity.GetComponent<LineRendererComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.Color), sizeof(comp.Color));
			p_Out.write(reinterpret_cast<const char*>(&comp.Width), sizeof(comp.Width));
			p_Out.write(reinterpret_cast<const char*>(&comp.Primitive), sizeof(comp.Primitive));
			p_Out.write(reinterpret_cast<const char*>(&comp.Start), sizeof(comp.Start));
			p_Out.write(reinterpret_cast<const char*>(&comp.End), sizeof(comp.End));
		}

		template<>
		void ComponentSerializeBinIfExist<TextRendererComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<TextRendererComponent>())
				return;

			Utils::WriteString(p_Out, "TextRendererComponent");

			auto& comp = p_Entity.GetComponent<TextRendererComponent>();

			Utils::WriteString(p_Out, comp.String);

			p_Out.write(reinterpret_cast<const char*>(&comp.Font), sizeof(comp.Font));
			p_Out.write(reinterpret_cast<const char*>(&comp.Color), sizeof(comp.Color));
			p_Out.write(reinterpret_cast<const char*>(&comp.BgColor), sizeof(comp.BgColor));
			p_Out.write(reinterpret_cast<const char*>(&comp.CharBgColor), sizeof(comp.CharBgColor));
			p_Out.write(reinterpret_cast<const char*>(&comp.DrawBg), sizeof(comp.DrawBg));
			p_Out.write(reinterpret_cast<const char*>(&comp.Kerning), sizeof(comp.Kerning));
			p_Out.write(reinterpret_cast<const char*>(&comp.LineSpacing), sizeof(comp.LineSpacing));
		}

		template<>
		void ComponentSerializeBinIfExist<Rigidbody2DComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<Rigidbody2DComponent>())
				return;

			Utils::WriteString(p_Out, "Rigidbody2DComponent");

			auto& comp = p_Entity.GetComponent<Rigidbody2DComponent>();
			auto bodyId = comp.Body;

			p_Out.write(reinterpret_cast<const char*>(&bodyId.Index), sizeof(bodyId.Index));
			p_Out.write(reinterpret_cast<const char*>(&bodyId.World), sizeof(bodyId.World));
			p_Out.write(reinterpret_cast<const char*>(&bodyId.Generation), sizeof(bodyId.Generation));
			p_Out.write(reinterpret_cast<const char*>(&comp.Type), sizeof(comp.Type));
			p_Out.write(reinterpret_cast<const char*>(&comp.FixedRotation), sizeof(comp.FixedRotation));
		}

		template<>
		void ComponentSerializeBinIfExist<BoxCollider2DComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (!p_Entity.HasComponent<BoxCollider2DComponent>())
				return;

			Utils::WriteString(p_Out, "BoxCollider2DComponent");

			auto& comp = p_Entity.GetComponent<BoxCollider2DComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_Out.write(reinterpret_cast<const char*>(&comp.Offset), sizeof(comp.Offset));
			p_Out.write(reinterpret_cast<const char*>(&comp.Size), sizeof(comp.Size));
			p_Out.write(reinterpret_cast<const char*>(&comp.Density), sizeof(comp.Density));
			p_Out.write(reinterpret_cast<const char*>(&comp.Friction), sizeof(comp.Friction));
			p_Out.write(reinterpret_cast<const char*>(&comp.Restitution), sizeof(comp.Restitution));
			p_Out.write(reinterpret_cast<const char*>(&comp.RestitutionThreshold), sizeof(comp.RestitutionThreshold));
		}

		template<>
		void ComponentSerializeBinIfExist<CircleCollider2DComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();
			if (!p_Entity.HasComponent<CircleCollider2DComponent>())
				return;

			Utils::WriteString(p_Out, "CircleCollider2DComponent");

			auto& comp = p_Entity.GetComponent<CircleCollider2DComponent>();

			p_Out.write(reinterpret_cast<const char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_Out.write(reinterpret_cast<const char*>(&comp.Offset), sizeof(comp.Offset));
			p_Out.write(reinterpret_cast<const char*>(&comp.Radius), sizeof(comp.Radius));
			p_Out.write(reinterpret_cast<const char*>(&comp.Density), sizeof(comp.Density));
			p_Out.write(reinterpret_cast<const char*>(&comp.Friction), sizeof(comp.Friction));
			p_Out.write(reinterpret_cast<const char*>(&comp.Restitution), sizeof(comp.Restitution));
			p_Out.write(reinterpret_cast<const char*>(&comp.RestitutionThreshold), sizeof(comp.RestitutionThreshold));
		}

		template<>
		void ComponentSerializeBinIfExist<ScriptComponent>(std::ofstream& p_Out, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

#define WRITE_SCRIPT_FIELD(FieldType, Type)									\
				case ScriptFieldType::FieldType:										\
				{																		\
					auto value = scriptField.GetValue<Type>();							\
					p_Out.write(reinterpret_cast<const char*>(&value), sizeof(Type));	\
				}																		\
				break

			if (!p_Entity.HasComponent<ScriptComponent>())
				return;

			Utils::WriteString(p_Out, "ScriptComponent");

			auto& comp = p_Entity.GetComponent<ScriptComponent>();

			Utils::WriteString(p_Out, comp.FullClassName);

			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(comp.FullClassName);
			const auto& fields = entityClass->GetFields();

			if (fields.size() > 0)
			{
				auto& entityFields = ScriptEngine::GetScriptFieldMap(p_Entity);

				int size = 0;
				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
						continue;

					if (field.IsPrivate && !field.Serialize)
						continue;

					size++;
				}
				p_Out.write(reinterpret_cast<const char*>(&size), sizeof(size));

				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
						continue;

					if (field.IsPrivate && !field.Serialize)
						continue;

					Utils::WriteString(p_Out, name);

					p_Out.write(reinterpret_cast<const char*>(&field.Type), sizeof(ScriptFieldType));

					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Float, float);
						WRITE_SCRIPT_FIELD(Double, double);
						WRITE_SCRIPT_FIELD(Bool, bool);
						WRITE_SCRIPT_FIELD(Char, char);
						WRITE_SCRIPT_FIELD(Byte, int8_t);
						WRITE_SCRIPT_FIELD(Short, int16_t);
						WRITE_SCRIPT_FIELD(Int, int32_t);
						WRITE_SCRIPT_FIELD(Long, int64_t);
						WRITE_SCRIPT_FIELD(UByte, uint8_t);
						WRITE_SCRIPT_FIELD(UShort, uint16_t);
						WRITE_SCRIPT_FIELD(UInt, uint32_t);
						WRITE_SCRIPT_FIELD(ULong, uint64_t);
						WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
						WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
						WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
						WRITE_SCRIPT_FIELD(Entity, UUID);
					}
				}
			}

#undef WRITE_SCRIPT_FIELD
		}

	} // namespace

	// Deserialize
	namespace
	{
		template<>
		void ComponentDeserializeBinIfExist<TagComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "TagComponent")
				return;

			auto& comp = p_Entity.GetComponent<TagComponent>();
			comp.Tag = Utils::ReadString(p_In);
		}

		template<>
		void ComponentDeserializeBinIfExist<HierarchyComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "HierarchyComponent")
				return;

			UUID parent, first, prev, next;
			p_In.read(reinterpret_cast<char*>(&parent), sizeof(UUID));
			p_In.read(reinterpret_cast<char*>(&first), sizeof(UUID));
			p_In.read(reinterpret_cast<char*>(&prev), sizeof(UUID));
			p_In.read(reinterpret_cast<char*>(&next), sizeof(UUID));

			auto& comp = p_Entity.AddOrReplaceComponent<HierarchyComponent>();
			auto scene = p_Entity.GetScene();
			comp.Parent = parent != 0ul ? scene->GetEntityByUUID(parent).GetHandle() : entt::null;
			comp.First = first != 0ul ? scene->GetEntityByUUID(first).GetHandle() : entt::null;
			comp.Prev = prev != 0ul ? scene->GetEntityByUUID(prev).GetHandle() : entt::null;
			comp.Next = next != 0ul ? scene->GetEntityByUUID(next).GetHandle() : entt::null;

			p_In.read(reinterpret_cast<char*>(&comp.ChildCount), sizeof(comp.ChildCount));
		}

		template<>
		void ComponentDeserializeBinIfExist<TransformComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "TransformComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<TransformComponent>();
			glm::vec3 translation, rotation, scale;
			p_In.read(reinterpret_cast<char*>(&translation), sizeof(translation));
			p_In.read(reinterpret_cast<char*>(&rotation), sizeof(rotation));
			p_In.read(reinterpret_cast<char*>(&scale), sizeof(scale));

			comp.SetLocalTranslation(translation);
			comp.SetLocalRotation(rotation);
			comp.SetLocalScale(scale);
		}

		template<>
		void ComponentDeserializeBinIfExist<CameraComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "CameraComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<CameraComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.Primary), sizeof(comp.Primary));
			p_In.read(reinterpret_cast<char*>(&comp.ClearColor), sizeof(comp.ClearColor));

			uint32_t width, height;
			p_In.read(reinterpret_cast<char*>(&width), sizeof(width));
			p_In.read(reinterpret_cast<char*>(&height), sizeof(height));

			bool isOrthographic, isAspectRatioFixed;
			p_In.read(reinterpret_cast<char*>(&isOrthographic), sizeof(isOrthographic));
			p_In.read(reinterpret_cast<char*>(&isAspectRatioFixed), sizeof(isAspectRatioFixed));

			float fov, farZ, nearZ, scale, zoom;
			p_In.read(reinterpret_cast<char*>(&fov), sizeof(fov));
			p_In.read(reinterpret_cast<char*>(&farZ), sizeof(farZ));
			p_In.read(reinterpret_cast<char*>(&nearZ), sizeof(nearZ));
			p_In.read(reinterpret_cast<char*>(&scale), sizeof(scale));
			p_In.read(reinterpret_cast<char*>(&zoom), sizeof(zoom));

			comp.Camera.SetViewportSize(width, height);
			comp.Camera.SetIsOrthographic(isOrthographic);
			comp.Camera.SetFixAspectRatio(isAspectRatioFixed);
			comp.Camera.SetFOV(fov);
			comp.Camera.SetFar(farZ);
			comp.Camera.SetNear(nearZ);
			comp.Camera.SetScale(scale);
			comp.Camera.SetZoom(zoom);
		}

		template<>
		void ComponentDeserializeBinIfExist<SpriteComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "SpriteComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<SpriteComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.Type), sizeof(comp.Type));
			p_In.read(reinterpret_cast<char*>(&comp.Color), sizeof(comp.Color));
			p_In.read(reinterpret_cast<char*>(&comp.Texture), sizeof(comp.Texture));
			p_In.read(reinterpret_cast<char*>(&comp.Thickness), sizeof(comp.Thickness));
			p_In.read(reinterpret_cast<char*>(&comp.Fade), sizeof(comp.Fade));
			p_In.read(reinterpret_cast<char*>(&comp.Size), sizeof(comp.Size));
			p_In.read(reinterpret_cast<char*>(&comp.BySize), sizeof(comp.BySize));
			p_In.read(reinterpret_cast<char*>(&comp.Offset), sizeof(comp.Offset));
			p_In.read(reinterpret_cast<char*>(&comp.Scale), sizeof(comp.Scale));
		}

		template<>
		void ComponentDeserializeBinIfExist<LineRendererComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "LineRendererComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<LineRendererComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.Color), sizeof(comp.Color));
			p_In.read(reinterpret_cast<char*>(&comp.Width), sizeof(comp.Width));
			p_In.read(reinterpret_cast<char*>(&comp.Primitive), sizeof(comp.Primitive));
			p_In.read(reinterpret_cast<char*>(&comp.Start), sizeof(comp.Start));
			p_In.read(reinterpret_cast<char*>(&comp.End), sizeof(comp.End));
		}

		template<>
		void ComponentDeserializeBinIfExist<TextRendererComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "TextRendererComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<TextRendererComponent>();

			comp.String = Utils::ReadString(p_In);
			p_In.read(reinterpret_cast<char*>(&comp.Font), sizeof(comp.Font));
			p_In.read(reinterpret_cast<char*>(&comp.Color), sizeof(comp.Color));
			p_In.read(reinterpret_cast<char*>(&comp.BgColor), sizeof(comp.BgColor));
			p_In.read(reinterpret_cast<char*>(&comp.CharBgColor), sizeof(comp.CharBgColor));
			p_In.read(reinterpret_cast<char*>(&comp.DrawBg), sizeof(comp.DrawBg));
			p_In.read(reinterpret_cast<char*>(&comp.Kerning), sizeof(comp.Kerning));
			p_In.read(reinterpret_cast<char*>(&comp.LineSpacing), sizeof(comp.LineSpacing));
		}

		template<>
		void ComponentDeserializeBinIfExist<Rigidbody2DComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "Rigidbody2DComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<Rigidbody2DComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.Body.Index), sizeof(comp.Body.Index));
			p_In.read(reinterpret_cast<char*>(&comp.Body.World), sizeof(comp.Body.World));
			p_In.read(reinterpret_cast<char*>(&comp.Body.Generation), sizeof(comp.Body.Generation));
			p_In.read(reinterpret_cast<char*>(&comp.Type), sizeof(comp.Type));
			p_In.read(reinterpret_cast<char*>(&comp.FixedRotation), sizeof(comp.FixedRotation));
		}

		template<>
		void ComponentDeserializeBinIfExist<BoxCollider2DComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "BoxCollider2DComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<BoxCollider2DComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_In.read(reinterpret_cast<char*>(&comp.Offset), sizeof(comp.Offset));
			p_In.read(reinterpret_cast<char*>(&comp.Size), sizeof(comp.Size));
			p_In.read(reinterpret_cast<char*>(&comp.Density), sizeof(comp.Density));
			p_In.read(reinterpret_cast<char*>(&comp.Friction), sizeof(comp.Friction));
			p_In.read(reinterpret_cast<char*>(&comp.Restitution), sizeof(comp.Restitution));
			p_In.read(reinterpret_cast<char*>(&comp.RestitutionThreshold), sizeof(comp.RestitutionThreshold));
		}

		template<>
		void ComponentDeserializeBinIfExist<CircleCollider2DComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

			if (p_Current != "CircleCollider2DComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<CircleCollider2DComponent>();

			p_In.read(reinterpret_cast<char*>(&comp.IsTrigger), sizeof(comp.IsTrigger));
			p_In.read(reinterpret_cast<char*>(&comp.Offset), sizeof(comp.Offset));
			p_In.read(reinterpret_cast<char*>(&comp.Radius), sizeof(comp.Radius));
			p_In.read(reinterpret_cast<char*>(&comp.Density), sizeof(comp.Density));
			p_In.read(reinterpret_cast<char*>(&comp.Friction), sizeof(comp.Friction));
			p_In.read(reinterpret_cast<char*>(&comp.Restitution), sizeof(comp.Restitution));
			p_In.read(reinterpret_cast<char*>(&comp.RestitutionThreshold), sizeof(comp.RestitutionThreshold));
		}

		template<>
		void ComponentDeserializeBinIfExist<ScriptComponent>(std::ifstream& p_In, const std::string& p_Current, entt::registry& p_Registry, Entity p_Entity)
		{
			KTN_PROFILE_FUNCTION();

#define READ_SCRIPT_FIELD(FieldType, Type)             \
				case ScriptFieldType::FieldType:                   \
				{                                                  \
					Type data;                                      \
					p_In.read(reinterpret_cast<char*>(&data), sizeof(Type)); \
					fieldInstance.SetValue(data);                  \
					break;                                         \
				}

			if (p_Current != "ScriptComponent")
				return;

			auto& comp = p_Entity.AddOrReplaceComponent<ScriptComponent>();
			comp.FullClassName = Utils::ReadString(p_In);

			auto entityClass = ScriptEngine::GetEntityClass(comp.FullClassName);
			const auto& fields = entityClass->GetFields();

			if (fields.size() > 0)
			{
				auto& entityFields = ScriptEngine::GetScriptFieldMap(p_Entity);

				int size = 0;
				p_In.read(reinterpret_cast<char*>(&size), sizeof(size));

				for (int i = 0; i < size; i++)
				{
					std::string name = Utils::ReadString(p_In);

					ScriptFieldType type;
					p_In.read(reinterpret_cast<char*>(&type), sizeof(type));

					ScriptFieldInstance& fieldInstance = entityFields[name];
					if (fields.find(name) == fields.end())
					{
						KTN_CORE_WARN("Script field '{}' not found in class '{}'. Skipping...", name, comp.FullClassName);
						continue;
					}
					fieldInstance.Field = fields.at(name);

					switch (type)
					{
						READ_SCRIPT_FIELD(Float, float);
						READ_SCRIPT_FIELD(Double, double);
						READ_SCRIPT_FIELD(Bool, bool);
						READ_SCRIPT_FIELD(Char, char);
						READ_SCRIPT_FIELD(Byte, int8_t);
						READ_SCRIPT_FIELD(Short, int16_t);
						READ_SCRIPT_FIELD(Int, int32_t);
						READ_SCRIPT_FIELD(Long, int64_t);
						READ_SCRIPT_FIELD(UByte, uint8_t);
						READ_SCRIPT_FIELD(UShort, uint16_t);
						READ_SCRIPT_FIELD(UInt, uint32_t);
						READ_SCRIPT_FIELD(ULong, uint64_t);
						READ_SCRIPT_FIELD(Vector2, glm::vec2);
						READ_SCRIPT_FIELD(Vector3, glm::vec3);
						READ_SCRIPT_FIELD(Vector4, glm::vec4);
						READ_SCRIPT_FIELD(Entity, UUID);
					}
				}
			}
			#undef READ_SCRIPT_FIELD
		}

	} // namespace

} // namespace KTN
