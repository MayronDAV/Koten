#pragma once
#include "Koten/Core/Buffer.h"
#include "Asset.h"
#include "Koten/Utils/HashCombiner.h"

// lib
#include <glm/glm.hpp>


namespace KTN
{
    struct AtlasRegion
    {
        std::string Name;

        glm::vec2 Position;
        glm::vec2 GridSize;

        glm::vec2 Pivot = { 0.5f, 0.5f };
        glm::vec4 UV    = { 0,0,1,1 };

        uint64_t ID     = 0;
    };

    class KTN_API TextureAtlas : public Asset
    {
        public:
            TextureAtlas() = default;
            ~TextureAtlas() = default;

            AssetHandle Texture = 0;
            std::vector<AtlasRegion> Regions;

            std::unordered_map<uint64_t, uint32_t> RegionMap;

            AtlasRegion& GetRegion(uint32_t p_Index)
            {
                return Regions[p_Index];
            }

            AtlasRegion* GetRegion(uint64_t p_ID)
            {
                if (RegionMap.empty())
                    BuildRegionMap();

                auto it = RegionMap.find(p_ID);
                if (it == RegionMap.end())
                    return nullptr;

                return &Regions[it->second];
            }

            uint32_t GetCount() const
            {
                return (uint32_t)Regions.size();
            }

            void BuildRegionMap()
            {
                if (Regions.empty()) return;

                //RegionMap.clear();
                RegionMap.reserve(Regions.size());
                for (size_t i = 0; i < Regions.size(); i++)
                {
                    Regions[i].ID               = HashString(Regions[i].Name);
                    RegionMap[Regions[i].ID]    = (uint32_t)i;
                }
            }

            ASSET_CLASS_METHODS(TextureAtlas)
    };

    class KTN_API TextureAtlasImporter
    {
        public:
            static Ref<TextureAtlas> Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
            static Ref<TextureAtlas> ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data);

            static Ref<TextureAtlas> Load(const std::string& p_Path);

            static void Save(const Ref<TextureAtlas>& p_Atlas, const std::string& p_Path);

            static void SaveBin(std::ofstream& p_Out, const Ref<TextureAtlas>& p_Atlas);
            static Ref<TextureAtlas> LoadBin(std::ifstream& p_In);
            static Ref<TextureAtlas> LoadBin(const Buffer& p_In);

            static void LoadBin(std::ifstream& p_In, Buffer& p_Buffer);
    };
} // namespace KTN