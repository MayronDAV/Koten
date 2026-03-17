#include "ktnpch.h"
#include "TextureAtlasImporter.h"

// lib
#include <yaml-cpp/yaml.h>



namespace KTN
{
    Ref<TextureAtlas> TextureAtlasImporter::Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::TextureAtlas)
        {
            KTN_CORE_ERROR("Invalid asset type for texture atlas import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        auto atlas = Load(p_Metadata.FilePath);

        return atlas;
    }

    Ref<TextureAtlas> TextureAtlasImporter::ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::TextureAtlas)
        {
            KTN_CORE_ERROR("Invalid asset type for texture atlas import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        auto atlas = LoadBin(p_Data);

        return atlas;
    }

    Ref<TextureAtlas> TextureAtlasImporter::Load(const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        if (FileSystem::GetExtension(p_Path) != ".ktatlas")
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
            KTN_CORE_ERROR("Failed to load .ktatlas file '{}'\n     {}", p_Path, e.what());
            return nullptr;
        }

        if (!data["AssetHandle"] || !data["Texture"])
        {
            KTN_CORE_ERROR("Failed to load file '{}'\n     Invalid texture atlas format!", p_Path);
            return nullptr;
        }

        auto atlas                 = CreateRef<TextureAtlas>();
        atlas->Handle              = data["AssetHandle"].as<uint64_t>();
        atlas->Texture             = data["Texture"].as<uint64_t>();

        const auto& regions        = data["Regions"];
        if (regions)
        {
            atlas->Regions.reserve(regions.size());
            for (const auto& regionData : regions)
            {
                AtlasRegion region = {};
                region.ID          = regionData["ID"].as<uint64_t>();
                region.Name        = regionData["Name"].as<std::string>();
                region.Position    = regionData["Position"].as<glm::vec2>();
                region.GridSize    = regionData["GridSize"].as<glm::vec2>();
                region.Pivot       = regionData["Pivot"].as<glm::vec2>();
                region.UV          = regionData["UV"].as<glm::vec4>();

                atlas->Regions.push_back(region);
            }

            atlas->BuildRegionLookup();
        }
        return atlas;
    }

    void TextureAtlasImporter::Save(const Ref<TextureAtlas>& p_TextureAtlas, const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "AssetHandle" << YAML::Value << p_TextureAtlas->Handle;
        out << YAML::Key << "Texture" << YAML::Value << p_TextureAtlas->Texture;

        out << YAML::Key << "Regions" << YAML::Value << YAML::BeginSeq;
        for (auto& region : p_TextureAtlas->Regions)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "ID" << YAML::Value << region.ID;
            out << YAML::Key << "Name" << YAML::Value << region.Name;
            out << YAML::Key << "Position" << YAML::Value << region.Position;
            out << YAML::Key << "GridSize" << YAML::Value << region.GridSize;
            out << YAML::Key << "Pivot" << YAML::Value << region.Pivot;
            out << YAML::Key << "UV" << YAML::Value << region.UV;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(p_Path);
        fout << out.c_str();

        KTN_CORE_INFO("Texture atlas serialized to path: {}", p_Path);
    }

    void TextureAtlasImporter::SaveBin(std::ofstream& p_Out, const Ref<TextureAtlas>& p_TextureAtlas)
    {
        KTN_PROFILE_FUNCTION();

        p_Out.write(reinterpret_cast<const char*>(&p_TextureAtlas->Handle), sizeof(AssetHandle));
        p_Out.write(reinterpret_cast<const char*>(&p_TextureAtlas->Texture), sizeof(AssetHandle));

        auto size = p_TextureAtlas->Regions.size();
        p_Out.write(reinterpret_cast<const char*>(&size), sizeof(size));

        for (const auto& region : p_TextureAtlas->Regions)
        {
            p_Out.write(reinterpret_cast<const char*>(&region.ID), sizeof(region.ID));
            Utils::WriteString(p_Out, region.Name);
            p_Out.write(reinterpret_cast<const char*>(&region.Position), sizeof(region.Position));
            p_Out.write(reinterpret_cast<const char*>(&region.GridSize), sizeof(region.GridSize));
            p_Out.write(reinterpret_cast<const char*>(&region.Pivot), sizeof(region.Pivot));
            p_Out.write(reinterpret_cast<const char*>(&region.UV), sizeof(region.UV));
        }
    }

    Ref<TextureAtlas> TextureAtlasImporter::LoadBin(std::ifstream& p_In)
    {
        KTN_PROFILE_FUNCTION();

        auto atlas = CreateRef<TextureAtlas>();

        p_In.read(reinterpret_cast<char*>(&atlas->Handle), sizeof(AssetHandle));
        p_In.read(reinterpret_cast<char*>(&atlas->Texture), sizeof(AssetHandle));

        size_t size = 0;
        p_In.read(reinterpret_cast<char*>(&size), sizeof(size));
        atlas->Regions.reserve(size);

        for (size_t i = 0; i < size; i++)
        {
            AtlasRegion region;
            p_In.read(reinterpret_cast<char*>(&region.ID), sizeof(region.ID));
            region.Name = Utils::ReadString(p_In);
            p_In.read(reinterpret_cast<char*>(&region.Position), sizeof(region.Position));
            p_In.read(reinterpret_cast<char*>(&region.GridSize), sizeof(region.GridSize));
            p_In.read(reinterpret_cast<char*>(&region.Pivot), sizeof(region.Pivot));
            p_In.read(reinterpret_cast<char*>(&region.UV), sizeof(region.UV));

            atlas->Regions.push_back(region);
        }

        atlas->BuildRegionLookup();

        return atlas;
    }

    Ref<TextureAtlas> TextureAtlasImporter::LoadBin(const Buffer& p_In)
    {
        KTN_PROFILE_FUNCTION();

        auto atlas = CreateRef<TextureAtlas>();

        BufferReader reader(p_In);

        reader.ReadBytes(&atlas->Handle, sizeof(AssetHandle));
        reader.ReadBytes(&atlas->Texture, sizeof(AssetHandle));

        size_t size = 0;
        reader.ReadBytes(&size, sizeof(size));

        for (size_t i = 0; i < size; i++)
        {
            AtlasRegion region;
            reader.ReadBytes(&region.ID, sizeof(region.ID));
            region.Name = Utils::ReadString(reader);
            reader.ReadBytes(&region.Position, sizeof(region.Position));
            reader.ReadBytes(&region.GridSize, sizeof(region.GridSize));
            reader.ReadBytes(&region.Pivot, sizeof(region.Pivot));
            reader.ReadBytes(&region.UV, sizeof(region.UV));

            atlas->Regions.push_back(region);
        }

        atlas->BuildRegionLookup();

        return atlas;
    }

    void TextureAtlasImporter::LoadBin(std::ifstream& p_In, Buffer& p_Buffer)
    {
        KTN_PROFILE_FUNCTION();

        AssetHandle handle;
        p_In.read(reinterpret_cast<char*>(&handle), sizeof(AssetHandle));
        p_Buffer.Write(&handle, sizeof(AssetHandle));

        AssetHandle texture;
        p_In.read(reinterpret_cast<char*>(&texture), sizeof(AssetHandle));
        p_Buffer.Write(&texture, sizeof(AssetHandle));

        size_t size = 0;
        p_In.read(reinterpret_cast<char*>(&size), sizeof(size));
        p_Buffer.Write(&size, sizeof(size));

        for (size_t i = 0; i < size; i++)
        {
            AtlasRegion region;

            p_In.read(reinterpret_cast<char*>(&region.ID), sizeof(region.ID));
            p_Buffer.Write(&region.ID, sizeof(region.ID));

            region.Name = Utils::ReadString(p_In);
            Utils::WriteString(p_Buffer, region.Name);

            p_In.read(reinterpret_cast<char*>(&region.Position), sizeof(region.Position));
            p_Buffer.Write(&region.Position, sizeof(region.Position));

            p_In.read(reinterpret_cast<char*>(&region.GridSize), sizeof(region.GridSize));
            p_Buffer.Write(&region.GridSize, sizeof(region.GridSize));

            p_In.read(reinterpret_cast<char*>(&region.Pivot), sizeof(region.Pivot));
            p_Buffer.Write(&region.Pivot, sizeof(region.Pivot));

            p_In.read(reinterpret_cast<char*>(&region.UV), sizeof(region.UV));
            p_Buffer.Write(&region.UV, sizeof(region.UV));
        }
    }

} // namespace KTN
