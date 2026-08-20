#include "ktnpch.h"
#include "AssetManager.h"
#include "AssetImporter.h"
#include "Koten/Project/Project.h"
#include "Koten/Graphics/DFFont.h"
#include "Koten/Scene/Entity.h"
#include "Koten/Utils/Utils.h"
#include "Koten/Scene/SceneSerializer.h"
#include "Koten/Physics/PhysicsMaterial2D.h"
#include "Koten/Asset/PrefabImporter.h"
#include "Koten/Scene/SceneManager.h"
#include "Koten/Asset/TextureAtlasImporter.h"
#include "Koten/Asset/AnimationImporter.h"
#include "Koten/Asset/AnimationControllerImporter.h"
#include "Koten/Asset/ShaderStage.h"
#include "Koten/Asset/ShaderImporter.h"

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

        auto& metadata = GetMetadata(p_Handle);
        if (m_Config.LoadAssetsFromPath || metadata.Type == AssetType::Font)
        {
            auto asset = AssetImporter::ImportAsset(p_Handle, metadata);
            if (!asset)
            {
                KTN_CORE_ERROR("AssetManager::GetAsset - Asset import failed!");
                return nullptr;
            }

            m_LoadedAssets[p_Handle] = asset;
            return asset;
        }
        else if (m_Config.LoadAssetsFromMemory)
        {
            auto asset = AssetImporter::ImportAssetFromMemory(p_Handle, metadata, m_AssetCache[p_Handle]->GetBuffer());
            if (!asset)
            {
                KTN_CORE_ERROR("AssetManager::GetAsset - Load Asset from memory failed!");
                return nullptr;
            }

            m_LoadedAssets[p_Handle] = asset;
            return asset;
        }

        KTN_CORE_ERROR("AssetManager::GetAsset - Something went wrong! LoadAssetsFromPath or LoadAssetsFromMemory must be true.");
        return nullptr;
    }

    Ref<Asset> AssetManager::LoadAsset(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
    {
        KTN_PROFILE_FUNCTION();

        if (!IsAssetHandleValid(p_Handle))
        {
            KTN_CORE_ERROR("AssetManager::LoadAsset - Invalid asset handle {}", (uint64_t)p_Handle);
            return nullptr;
        }

        if ((m_Config.LoadAssetsFromPath) || p_Metadata.Type == AssetType::Font)
        {
            auto asset                          = AssetImporter::ImportAsset(p_Handle, p_Metadata);
            if (!asset)
            {
                KTN_CORE_ERROR("AssetManager::LoadAsset - Asset import failed!");
                return nullptr;
            }

            if (p_Metadata.Scope == AssetScope::Project)
            {
                delete m_AssetRegistry[p_Handle].AssetData;
                m_AssetRegistry[p_Handle].AssetData       = nullptr;
                m_AssetRegistry[p_Handle]                 = p_Metadata;
            }
            else if (p_Metadata.Scope == AssetScope::Global)
            {
                delete m_GlobalAssetRegistry[p_Handle].AssetData;
                m_GlobalAssetRegistry[p_Handle].AssetData = nullptr;
                m_GlobalAssetRegistry[p_Handle]           = p_Metadata;
            }

            m_LoadedAssets[p_Handle]                      = asset;
            return asset;
        }
        else if (m_Config.LoadAssetsFromMemory)
        {
            auto asset                          = AssetImporter::ImportAssetFromMemory(p_Handle, p_Metadata, m_AssetCache[p_Handle]->GetBuffer());
            if (!asset)
            {
                KTN_CORE_ERROR("AssetManager::LoadAsset - Load Asset from memory failed!");
                return nullptr;
            }

            if (p_Metadata.Scope == AssetScope::Project)
            {
                delete m_AssetRegistry[p_Handle].AssetData;
                m_AssetRegistry[p_Handle].AssetData       = nullptr;
                m_AssetRegistry[p_Handle]                 = p_Metadata;
            }
            else if (p_Metadata.Scope == AssetScope::Global)
            {
                delete m_GlobalAssetRegistry[p_Handle].AssetData;
                m_GlobalAssetRegistry[p_Handle].AssetData = nullptr;
                m_GlobalAssetRegistry[p_Handle]           = p_Metadata;
            }

            m_LoadedAssets[p_Handle]                      = asset;
            return asset;
        }

        KTN_CORE_ERROR("AssetManager::LoadAsset - Something went wrong! LoadAssetsFromPath or LoadAssetsFromMemory must be true.");
        return nullptr;
    }

    AssetHandle AssetManager::GetHandleByPath(const std::string& p_FilePath) const
    {
        KTN_PROFILE_FUNCTION();

        if (!m_GlobalAssetRegistry.empty())
        {
            auto it = std::find_if(m_GlobalAssetRegistry.begin(), m_GlobalAssetRegistry.end(),
            [&p_FilePath](const auto& p_Pair)
            {
                return p_Pair.second.FilePath == p_FilePath;
            });

            if (it != m_GlobalAssetRegistry.end())
                return it->first;
        }

        if (!m_AssetRegistry.empty())
        {
            auto it = std::find_if(m_AssetRegistry.begin(), m_AssetRegistry.end(),
            [&p_FilePath](const auto& p_Pair)
            {
                return p_Pair.second.FilePath == p_FilePath;
            });

            if (it != m_AssetRegistry.end())
                return it->first;
        }

        return (AssetHandle)0;
    }

    AssetType AssetManager::GetAssetType(AssetHandle p_Handle) const
    {
        KTN_PROFILE_FUNCTION();

        return GetMetadata(p_Handle).Type;
    }

    void AssetManager::Init(const AssetManagerConfig& p_Config)
    {
        KTN_PROFILE_FUNCTION();

        auto assetManager    = CreateRef<AssetManager>();
        s_Instance           = assetManager;
        s_Instance->m_Config = p_Config;

        if (p_Config.LoadAssetsFromMemory)
        {
            bool success = s_Instance->DeserializeAssetPack();
            KTN_VERIFY(success, "Failed to load asset pack!");
        }

        auto& settings = Engine::Get().GetSettings();
        if (settings.ReadGlobalFiles && p_Config.LoadAssetsFromPath)
        {
            s_Instance->DeserializeAssetRegistry();

            auto isStageFile = [](const std::filesystem::path& p_Path)
        {
            auto ext = p_Path.extension();

            return ext == ".vertex"   || ext == ".vert" ||
                   ext == ".fragment" || ext == ".frag" ||
                   ext == ".geometry" || ext == ".geom" ||
                   ext == ".tesc"     || ext == ".tese";
        };

            for (const auto& entry : std::filesystem::recursive_directory_iterator("Assets/Shaders"))
            {
                auto& path                      = entry.path();
                if (!std::filesystem::is_regular_file(path)) continue;

                auto filePath                   = FileSystem::GetRelative(path.string(), "Assets");
                if (isStageFile(path))
                {
                    AssetMetadata metadata      = {};
                    metadata.Type               = AssetType::ShaderStage;
                    metadata.FilePath           = filePath;
                    metadata.SerializeAssetData = false;
                    metadata.Scope              = AssetScope::Global;

                    AssetManager::Get()->ImportAsset(metadata);
                }

                if (path.extension() == ".ktshader")
                {
                    AssetMetadata metadata      = {};
                    metadata.Type               = AssetType::Shader;
                    metadata.FilePath           = "Assets\\" + filePath;
                    metadata.SerializeAssetData = false;
                    metadata.Load               = false;
                    metadata.Scope              = AssetScope::Global;

                    AssetManager::Get()->ImportAsset(metadata);
                }
            }
        }
    }

    void AssetManager::SetConfig(const AssetManagerConfig& p_Config)
    {
        KTN_PROFILE_FUNCTION();

        s_Instance->m_Config = p_Config;
    }

    AssetManager::~AssetManager()
    {
        KTN_PROFILE_FUNCTION();

        for (auto& [handle, metadata] : m_GlobalAssetRegistry)
        {
            if (metadata.AssetData != nullptr)
            {
                delete metadata.AssetData;
                metadata.AssetData = nullptr;
            }
        }
        m_GlobalAssetRegistry.clear();

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

        AssetMetadata metadata = {};
        metadata.Type          = p_Type;
        metadata.FilePath      = p_FilePath;
        return ImportAsset(metadata, p_Force);
    }

    AssetHandle AssetManager::ImportAsset(const AssetMetadata& p_Metadata, bool p_Force)
    {
        KTN_PROFILE_FUNCTION();

        AssetMetadata metadata = p_Metadata;
        if (metadata.Load)
        {
            if (metadata.Scope == AssetScope::Project)
                metadata.FilePath = (Project::GetAssetDirectory() / FileSystem::GetRelative(metadata.FilePath, Project::GetAssetDirectory().string())).string();
            else if (metadata.Scope == AssetScope::Global)
            {
                metadata.FilePath = "Assets/" + metadata.FilePath;
            }
        }
        metadata.FilePath = FileSystem::NormalizePath(metadata.FilePath);

        if (auto handle = GetHandleByPath(metadata.FilePath);
            IsAssetHandleValid(handle) && !p_Force)
        {
            return handle;
        }

        AssetHandle handle; // generate new handle
        Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);

        if (asset)
        {
            asset->Handle          = handle;
            m_LoadedAssets[handle] = asset;

            if (metadata.Scope == AssetScope::Project)
                m_AssetRegistry[handle] = metadata;
            else if (metadata.Scope == AssetScope::Global)
                m_GlobalAssetRegistry[handle] = metadata;

            if (!m_IsLoadedAssetPack)
                SerializeAssetRegistry();
            else
                m_NeedsToUpdate = true;

            return handle;
        }

        KTN_CORE_ERROR("Failed to import asset: {}, {}", GetAssetTypeName(metadata.Type), metadata.FilePath);
        return 0;
    }

    bool AssetManager::ImportAsset(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Ref<Asset>& p_Asset)
    {
        KTN_PROFILE_FUNCTION();

        Ref<Asset> asset = p_Asset;
        AssetHandle handle = p_Handle != 0 ? p_Handle : AssetHandle();
        if (!asset)
            asset = AssetImporter::ImportAsset(handle, p_Metadata);

        AssetMetadata metadata = p_Metadata;
        if (metadata.Load)
        {
            if (metadata.Scope == AssetScope::Project)
                metadata.FilePath = (Project::GetAssetDirectory() / FileSystem::GetRelative(metadata.FilePath, Project::GetAssetDirectory().string())).string();
            else if (metadata.Scope == AssetScope::Global)
            {
                metadata.FilePath = "Assets/" + metadata.FilePath;
            }
        }
        metadata.FilePath = FileSystem::NormalizePath(metadata.FilePath);

        if (asset)
        {
            asset->Handle = handle;
            m_LoadedAssets[handle] = asset;

            if (metadata.Scope == AssetScope::Project)
                m_AssetRegistry[handle] = metadata;
            else if (metadata.Scope == AssetScope::Global)
                m_GlobalAssetRegistry[handle] = metadata;

            if (!m_IsLoadedAssetPack)
                SerializeAssetRegistry();
            else
                m_NeedsToUpdate = true;
            return true;
        }

        KTN_CORE_ERROR("Failed to import asset: {}, {}", GetAssetTypeName(metadata.Type), metadata.FilePath);
        return false;
    }

    bool AssetManager::IsAssetHandleValid(AssetHandle p_Handle) const
    {
        KTN_PROFILE_FUNCTION();

        return p_Handle != 0 && (m_GlobalAssetRegistry.find(p_Handle) != m_GlobalAssetRegistry.end() || m_AssetRegistry.find(p_Handle) != m_AssetRegistry.end());
    }

    bool AssetManager::IsAssetLoaded(AssetHandle p_Handle) const
    {
        KTN_PROFILE_FUNCTION();

        return p_Handle != 0 && m_LoadedAssets.find(p_Handle) != m_LoadedAssets.end();
    }

    bool AssetManager::HasAsset(AssetType p_Type, const std::string& p_FilePath) const
    {
        KTN_PROFILE_FUNCTION();

        if (!m_GlobalAssetRegistry.empty())
        {
            auto it = std::find_if(m_GlobalAssetRegistry.begin(), m_GlobalAssetRegistry.end(),
            [&p_Type, &p_FilePath](const auto& p_Pair)
            {
                return p_Pair.second.Type == p_Type && p_Pair.second.FilePath == p_FilePath;
            });

            return it != m_GlobalAssetRegistry.end();
        }

        if (!m_AssetRegistry.empty())
        {
            auto it = std::find_if(m_AssetRegistry.begin(), m_AssetRegistry.end(),
            [&p_Type, &p_FilePath](const auto& p_Pair)
            {
                return p_Pair.second.Type == p_Type && p_Pair.second.FilePath == p_FilePath;
            });

            return it != m_AssetRegistry.end();
        }

        return false;
    }

    bool AssetManager::RemoveAsset(AssetHandle p_Handle)
    {
        KTN_PROFILE_FUNCTION();

        if (!m_GlobalAssetRegistry.empty())
        {
            auto it = m_GlobalAssetRegistry.find(p_Handle);
            if (it != m_GlobalAssetRegistry.end())
            {
                if (m_LoadedAssets.find(p_Handle) != m_LoadedAssets.end())
                    m_LoadedAssets.erase(p_Handle);

                auto& metadata = it->second;
                if (metadata.AssetData != nullptr)
                {
                    delete metadata.AssetData;
                    metadata.AssetData = nullptr;
                }

                m_GlobalAssetRegistry.erase(p_Handle);

                if (!m_IsLoadedAssetPack)
                    SerializeAssetRegistry();
                else
                    SerializeAssetPack();
                return true;
            }
        }

        if (!m_AssetRegistry.empty())
        {
            auto it = m_AssetRegistry.find(p_Handle);
            if (it != m_AssetRegistry.end())
            {
                if (m_LoadedAssets.find(p_Handle) != m_LoadedAssets.end())
                    m_LoadedAssets.erase(p_Handle);

                auto& metadata = it->second;
                if (metadata.AssetData != nullptr)
                {
                    delete metadata.AssetData;
                    metadata.AssetData = nullptr;
                }

                m_AssetRegistry.erase(p_Handle);

                if (!m_IsLoadedAssetPack)
                    SerializeAssetRegistry();
                else
                    SerializeAssetPack();
                return true;
            }
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

        if (!m_GlobalAssetRegistry.empty())
        {
            auto it = m_GlobalAssetRegistry.find(p_Handle);
            if (it != m_GlobalAssetRegistry.end())
                return it->second;
        }

        return m_AssetRegistry.at(p_Handle);
    }

    AssetMetadata& AssetManager::GetMetadata(AssetHandle p_Handle)
    {
        KTN_PROFILE_FUNCTION();

        static AssetMetadata s_EmptyMetadata;
        if (!IsAssetHandleValid(p_Handle))
            return s_EmptyMetadata;

        if (!m_GlobalAssetRegistry.empty())
        {
            auto it = m_GlobalAssetRegistry.find(p_Handle);
            if (it != m_GlobalAssetRegistry.end())
                return it->second;
        }

        return m_AssetRegistry.at(p_Handle);
    }

    struct AssetPackHeader
    {
        char Magic[4]                  = { 'K', 'T', 'A', 'P' };
        uint32_t Version               = 2;
        size_t GlobalAssetRegistrySize = 0;
        size_t AssetRegistrySize       = 0;
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
        header.GlobalAssetRegistrySize = m_GlobalAssetRegistry.size();
        header.AssetRegistrySize       = m_AssetRegistry.size();

        out.write(reinterpret_cast<const char*>(&header), sizeof(header));

        auto serialize = [&](const AssetHandle& p_Handle, const AssetMetadata& p_Metadata)
        {
            out.write(reinterpret_cast<const char*>(&p_Handle), sizeof(p_Handle));
            out.write(reinterpret_cast<const char*>(&p_Metadata.Type), sizeof(p_Metadata.Type));
            auto path = p_Metadata.FilePath;
            if (p_Metadata.Load)
            {
                if (p_Metadata.Scope == AssetScope::Project)
                    path = FileSystem::GetRelative(p_Metadata.FilePath, Project::GetAssetDirectory().string());
            }
            Utils::WriteString(out, FileSystem::NormalizePath(path));

            out.write(reinterpret_cast<const char*>(&p_Metadata.Load), sizeof(p_Metadata.Load));
            out.write(reinterpret_cast<const char*>(&p_Metadata.SerializeAssetData), sizeof(p_Metadata.SerializeAssetData));
            out.write(reinterpret_cast<const char*>(&p_Metadata.Scope), sizeof(p_Metadata.Scope));

            if (p_Metadata.SerializeAssetData)
            {
                switch (p_Metadata.Type)
                {
                    case AssetType::Font:
                    {
                        auto dffont = GetAsset<DFFont>(p_Handle);
                        const auto& config = dffont->GetConfig();

                        out.write(reinterpret_cast<const char*>(&config.ImageType), sizeof(config.ImageType));
                        out.write(reinterpret_cast<const char*>(&config.GlyphIdentifier), sizeof(config.GlyphIdentifier));
                        out.write(reinterpret_cast<const char*>(&config.ImageFormat), sizeof(config.ImageFormat));
                        out.write(reinterpret_cast<const char*>(&config.EmSize), sizeof(config.EmSize));
                        out.write(reinterpret_cast<const char*>(&config.PxRange), sizeof(config.PxRange));
                        out.write(reinterpret_cast<const char*>(&config.MiterLimit), sizeof(config.MiterLimit));
                        out.write(reinterpret_cast<const char*>(&config.AngleThreshold), sizeof(config.AngleThreshold));
                        out.write(reinterpret_cast<const char*>(&config.FontScale), sizeof(config.FontScale));
                        out.write(reinterpret_cast<const char*>(&config.ThreadCount), sizeof(config.ThreadCount));
                        out.write(reinterpret_cast<const char*>(&config.ExpensiveColoring), sizeof(config.ExpensiveColoring));
                        out.write(reinterpret_cast<const char*>(&config.FixedScale), sizeof(config.FixedScale));
                        out.write(reinterpret_cast<const char*>(&config.OverlapSupport), sizeof(config.OverlapSupport));
                        out.write(reinterpret_cast<const char*>(&config.ScanlinePass), sizeof(config.ScanlinePass));
                        out.write(reinterpret_cast<const char*>(&config.UseDefaultCharset), sizeof(config.UseDefaultCharset));

                        if (!config.UseDefaultCharset)
                        {
                            size_t numRanges = config.CharsetRanges.size();
                            out.write(reinterpret_cast<const char*>(&numRanges), sizeof(numRanges));

                            for (const auto& range : config.CharsetRanges)
                            {
                                out.write(reinterpret_cast<const char*>(&range.first), sizeof(uint32_t));
                                out.write(reinterpret_cast<const char*>(&range.second), sizeof(uint32_t));
                            }
                        }
                        break;
                    }
                    case AssetType::Texture2D:
                    {
                        auto texture = GetAsset<Texture2D>(p_Handle);
                        WriteTextureSpecification(out, texture->GetSpecification());
                        break;
                    }
                }
            }

            if (p_Metadata.Type == AssetType::ShaderStage)
            {
                auto shaderStage = GetAsset<ShaderStage>(p_Handle);

                ShaderStageImporter::SaveBin(out, shaderStage);
            }

            if (p_Metadata.Type == AssetType::Shader)
            {
                auto shader = GetAsset<Shader>(p_Handle);

                ShaderImporter::Save(out, shader);
            }

            if (p_Metadata.Type == AssetType::Texture2D)
            {
                auto texture = GetAsset<Texture2D>(p_Handle);

                bool renderTarget = texture->IsColorAttachment();
                out.write(reinterpret_cast<const char*>(&renderTarget), sizeof(renderTarget));

                bool hasData = !renderTarget && texture;
                out.write(reinterpret_cast<const char*>(&hasData), sizeof(hasData));

                if (hasData)
                {
                    std::vector<uint8_t> textureData = texture->GetData();
                    size_t dataSize = textureData.size();

                    out.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
                    out.write(reinterpret_cast<const char*>(textureData.data()), dataSize);
                }
            }

            if (p_Metadata.Type == AssetType::Scene)
            {
                auto scene = GetAsset<Scene>(p_Handle);
                KTN_CORE_ASSERT(scene, "Scene is nullptr!");

                SceneSerializer serializer(scene);
                serializer.SerializeBin(out);
            }

            if (p_Metadata.Type == AssetType::PhysicsMaterial2D)
            {
                auto material = GetAsset<PhysicsMaterial2D>(p_Handle);
                KTN_CORE_ASSERT(material, "PhysicsMaterial2D is nullptr!");

                material->SerializeBin(out);
            }

            if (p_Metadata.Type == AssetType::Prefab)
            {
                auto prefab = PrefabImporter::Load(SceneManager::GetActiveScenes()[0]->Handle, p_Metadata.FilePath);
                KTN_CORE_ASSERT(prefab, "Prefab is nullptr!");

                PrefabImporter::SaveBin(out, prefab);

                prefab->Entt.Destroy();
            }

            if (p_Metadata.Type == AssetType::Material)
            {
                auto material = GetAsset<Material>(p_Handle);
                KTN_CORE_ASSERT(material, "Material is nullptr!");

                material->SerializeBin(out);
            }

            if (p_Metadata.Type == AssetType::TextureAtlas)
            {
                auto atlas = GetAsset<TextureAtlas>(p_Handle);
                KTN_CORE_ASSERT(atlas, "TextureAtlas is nullptr!");
                TextureAtlasImporter::SaveBin(out, atlas);
            }

            if (p_Metadata.Type == AssetType::Animation)
            {
                auto anim = GetAsset<Animation>(p_Handle);
                KTN_CORE_ASSERT(anim, "Animation is nullptr!");
                AnimationImporter::SaveBin(out, anim);
            }

            if (p_Metadata.Type == AssetType::AnimationController)
            {
                auto controller = GetAsset<AnimationController>(p_Handle);
                KTN_CORE_ASSERT(controller, "AnimationController is nullptr!");
                AnimationControllerImporter::SaveBin(out, controller);
            }
        };

        for (auto& [handle, metadata] : m_GlobalAssetRegistry)
        {
            serialize(handle, metadata);
        }

        for (auto& [handle, metadata] : m_AssetRegistry)
        {
            serialize(handle, metadata);
        }
    }

    bool AssetManager::DeserializeAssetPack(const std::filesystem::path& p_Folder)
    {
        KTN_PROFILE_FUNCTION();

        const auto cachePath = p_Folder / "AssetPack.ktap";
        m_IsLoadedAssetPack  = true;
        m_NeedsToUpdate      = false;

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

        #define    READ_WRITE(dest, size)                      \
            in.read(reinterpret_cast<char*>(dest), size); \
            buffer.Write(dest, size);

        auto deserialize = [&]()
        {
            AssetHandle handle;
            AssetMetadata metadata;

            in.read(reinterpret_cast<char*>(&handle), sizeof(handle));
            in.read(reinterpret_cast<char*>(&metadata.Type), sizeof(metadata.Type));
            auto path         = Utils::ReadString(in);

            Buffer buffer = {};

            metadata.Load = false;
            in.read(reinterpret_cast<char*>(&metadata.Load), sizeof(metadata.Load));

            metadata.SerializeAssetData = false;
            in.read(reinterpret_cast<char*>(&metadata.SerializeAssetData), sizeof(metadata.SerializeAssetData));

            metadata.Scope = AssetScope::Project;
            in.read(reinterpret_cast<char*>(&metadata.Scope), sizeof(metadata.Scope));

            metadata.FilePath = path;
            if (metadata.Load)
            {
                if (metadata.Scope == AssetScope::Project)
                    metadata.FilePath = (Project::GetAssetDirectory() / metadata.FilePath).string();
            }
            metadata.FilePath = FileSystem::NormalizePath(metadata.FilePath);

            if (metadata.SerializeAssetData)
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

            if (metadata.Type == AssetType::ShaderStage)
            {
                ShaderStageImporter::LoadBin(in, buffer);
            }

            if (metadata.Type == AssetType::Shader)
            {
                ShaderImporter::LoadBin(in, buffer);
            }

            if (metadata.Type == AssetType::Texture2D)
            {
                bool renderTarget = false;
                READ_WRITE(&renderTarget, sizeof(renderTarget));

                bool hasData = false;
                READ_WRITE(&hasData, sizeof(hasData));

                if (hasData)
                {
                    size_t dataSize = 0;
                    READ_WRITE(&dataSize, sizeof(dataSize));

                    std::vector<uint8_t> textureData(dataSize);
                    READ_WRITE(textureData.data(), dataSize);
                }
            }

            if (metadata.Type == AssetType::Scene)
            {
                SceneSerializer::DeserializeBin(in, buffer);
            }

            if (metadata.Type == AssetType::PhysicsMaterial2D)
            {
                PhysicsMaterial2D::DeserializeBin(in, buffer);
            }

            if (metadata.Type == AssetType::Prefab)
            {
                PrefabImporter::LoadBin(in, buffer);
            }

            if (metadata.Type == AssetType::Material)
            {
                Material::DeserializeBin(in, buffer);
            }

            if (metadata.Type == AssetType::TextureAtlas)
            {
                TextureAtlasImporter::LoadBin(in, buffer);
            }

            if (metadata.Type == AssetType::Animation)
            {
                AnimationImporter::LoadBin(in, buffer);
            }

            if (metadata.Type == AssetType::AnimationController)
            {
                AnimationControllerImporter::LoadBin(in, buffer);
            }

            m_AssetCache[handle] = CreateRef<ScopedBuffer>(buffer);
            if (metadata.Scope == AssetScope::Project)
                m_AssetRegistry[handle] = metadata;
            else if (metadata.Scope == AssetScope::Global)
                m_GlobalAssetRegistry[handle] = metadata;
        };

        for (size_t i = 0; i < header.GlobalAssetRegistrySize; ++i)
        {
            deserialize();
        }

        for (size_t i = 0; i < header.AssetRegistrySize; ++i)
        {
            deserialize();
        }

        #undef READ_WRITE

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

        auto serialize = [&](YAML::Emitter& out, const AssetHandle& handle, const AssetMetadata& metadata)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Handle" << YAML::Value << handle;
            out << YAML::Key << "FilePath" << YAML::Value << FileSystem::NormalizePath((metadata.Load ? FileSystem::GetRelative(metadata.FilePath, metadata.Scope == AssetScope::Project ? Project::GetAssetDirectory().string() : "Assets") : metadata.FilePath));
            out << YAML::Key << "Type" << YAML::Value << (std::string)GetAssetTypeName(metadata.Type);
            out << YAML::Key << "Load" << YAML::Value << metadata.Load;
            out << YAML::Key << "SerializeAssetData" << YAML::Value << metadata.SerializeAssetData;
            out << YAML::Key << "Scope" << YAML::Value << (std::string)GetAssetScopeName(metadata.Scope);
            if (metadata.SerializeAssetData)
            {
                out << YAML::Key << "AssetData" << YAML::Value << YAML::BeginMap;
                if (metadata.Type == AssetType::Font)
                {
                    auto dffont = GetAsset<DFFont>(handle);
                    const auto& config = dffont->GetConfig();
                    out << YAML::Key << "ImageType" << YAML::Value << (std::string)magic_enum::enum_name(config.ImageType).data();
                    out << YAML::Key << "GlyphIdentifier" << YAML::Value << (std::string)magic_enum::enum_name(config.GlyphIdentifier).data();
                    out << YAML::Key << "ImageFormat" << YAML::Value << (std::string)magic_enum::enum_name(config.ImageFormat).data();
                    out << YAML::Key << "EmSize" << YAML::Value << config.EmSize;
                    out << YAML::Key << "PxRange" << YAML::Value << config.PxRange;
                    out << YAML::Key << "MiterLimit" << YAML::Value << config.MiterLimit;
                    out << YAML::Key << "AngleThreshold" << YAML::Value << config.AngleThreshold;
                    out << YAML::Key << "FontScale" << YAML::Value << config.FontScale;
                    out << YAML::Key << "ThreadCount" << YAML::Value << config.ThreadCount;
                    out << YAML::Key << "ExpensiveColoring" << YAML::Value << config.ExpensiveColoring;
                    out << YAML::Key << "FixedScale" << YAML::Value << config.FixedScale;
                    out << YAML::Key << "OverlapSupport" << YAML::Value << config.OverlapSupport;
                    out << YAML::Key << "ScanlinePass" << YAML::Value << config.ScanlinePass;
                    out << YAML::Key << "UseDefaultCharset" << YAML::Value << config.UseDefaultCharset;
                    if (!config.UseDefaultCharset)
                    {
                        out << YAML::Key << "CharsetRanges" << YAML::Value;
                        out << YAML::BeginSeq;
                        for (const auto& range : config.CharsetRanges)
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
                    auto texture = GetAsset<Texture2D>(handle);
                    const auto& spec = texture->GetSpecification();
                    out << YAML::Key << "MinFilter" << YAML::Value << (std::string)magic_enum::enum_name(spec.MinFilter);
                    out << YAML::Key << "MagFilter" << YAML::Value << (std::string)magic_enum::enum_name(spec.MagFilter);
                    out << YAML::Key << "WrapU" << YAML::Value << (std::string)magic_enum::enum_name(spec.WrapU);
                    out << YAML::Key << "WrapV" << YAML::Value << (std::string)magic_enum::enum_name(spec.WrapV);
                    out << YAML::Key << "SRGB" << YAML::Value << spec.SRGB;
                    out << YAML::Key << "AnisotropyEnable" << YAML::Value << spec.AnisotropyEnable;
                    out << YAML::Key << "GenerateMipmaps" << YAML::Value << spec.GenerateMips;
                    out << YAML::Key << "BorderColor" << YAML::Value << spec.BorderColor;
                    out << YAML::Key << "DebugName" << YAML::Value << spec.DebugName;
                }

                out << YAML::EndMap;
            }
            out << YAML::EndMap;
        };

        if (!m_GlobalAssetRegistry.empty())
        {
            auto path = "Assets/GlobalAssetRegistry.ktreg";

            YAML::Emitter out;
            {
                out << YAML::BeginMap; // Root
                out << YAML::Key << "GlobalAssetRegistry" << YAML::Value;

                out << YAML::BeginSeq;
                for (const auto& [handle, metadata] : m_GlobalAssetRegistry)
                {
                    serialize(out, handle, metadata);
                }
                out << YAML::EndSeq;
                out << YAML::EndMap; // Root
            }

            std::ofstream fout(path);
            fout << out.c_str();
        }

        if (!m_AssetRegistry.empty())
        {
            auto path = Project::GetAssetDirectory() / "AssetRegistry.ktreg";

            YAML::Emitter out;
            {
                out << YAML::BeginMap; // Root
                out << YAML::Key << "AssetRegistry" << YAML::Value;

                out << YAML::BeginSeq;
                for (const auto& [handle, metadata] : m_AssetRegistry)
                {
                    serialize(out, handle, metadata);
                }
                out << YAML::EndSeq;
                out << YAML::EndMap; // Root
            }

            std::ofstream fout(path);
            fout << out.c_str();
        }
    }

    bool AssetManager::DeserializeAssetRegistry()
    {
        KTN_PROFILE_FUNCTION();

        auto deserialize = [&](YAML::Node& rootNode)
        {
            for (const auto& node : rootNode)
            {
                AssetHandle handle          = node["Handle"].as<uint64_t>();

                auto path                   = node["FilePath"].as<std::string>();
                auto type                   = node["Type"].as<std::string>();
                auto load                   = node["Load"].as<bool>();
                auto serialize              = node["SerializeAssetData"].as<bool>();
                auto scope                  = GetAssetScopeFromName(node["Scope"].as<std::string>().c_str());

                auto& metadata              = scope == AssetScope::Project ? m_AssetRegistry[handle] : m_GlobalAssetRegistry[handle];
                metadata.Load               = load;
                metadata.SerializeAssetData = serialize;
                metadata.Type               = GetAssetTypeFromName(type.c_str());
                metadata.Scope              = scope;

                metadata.FilePath           = path;
                if (metadata.Load)
                {
                    if (metadata.Scope == AssetScope::Project)
                        metadata.FilePath   = (Project::GetAssetDirectory() / metadata.FilePath).string();
                    if (metadata.Scope == AssetScope::Global)
                        metadata.FilePath   = "Assets/" + metadata.FilePath;
                }
                metadata.FilePath           = FileSystem::NormalizePath(metadata.FilePath);

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

                if (scope == AssetScope::Global && metadata.Load)
                {
                    LoadAsset(handle, metadata);
                }
            }
        };

        auto globalPath = "Assets/GlobalAssetRegistry.ktreg";
        if (m_Config.LoadGlobalAssetRegistry && FileSystem::Exists(globalPath))
        {
            bool success = true;
            YAML::Node data;
            try
            {
                data = YAML::LoadFile(globalPath);
            }
            catch (YAML::ParserException e)
            {
                KTN_CORE_ERROR("Failed to load global asset registry file '{0}'\n     {1}", globalPath, e.what());
                success = false;
            }
            if (success)
            {
                auto rootNode = data["GlobalAssetRegistry"];
                if (rootNode)
                    deserialize(rootNode);
            }
        }

        if (m_Config.LoadProjectAssetRegistry)
        {
            auto path = Project::GetAssetDirectory() / "AssetRegistry.ktreg";
            if (FileSystem::Exists(path.string()))
            {
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
                if (rootNode)
                    deserialize(rootNode);
            }
        }

        return true;
    }

} // namespace KTN
