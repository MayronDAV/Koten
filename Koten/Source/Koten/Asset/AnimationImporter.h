#pragma once
#include "Koten/Core/Buffer.h"
#include "Asset.h"
#include "Koten/Utils/HashCombiner.h"

// lib
#include <glm/glm.hpp>



namespace KTN
{
    struct AnimationFrame
    {
        uint64_t AtlasRegionID = 0;
        float Time             = 0.0f;
    };

    enum class AnimationMode
    {
        Loop,
        PingPong,
        Once
    };

    struct AnimationClip
    {
        std::string Name    = "None";

        std::vector<AnimationFrame> Frames;

        AnimationMode Mode  = AnimationMode::Loop;
        float Speed         = 1.0f;
        float TotalDuration = 1.0f;
        uint64_t ID         = 0;

        uint32_t GetCount() const
        {
            return (uint32_t)Frames.size();
        }
    };

    class KTN_API Animation : public Asset
    {
        public:
            Animation() = default;
            ~Animation() = default;

            std::vector<AnimationClip> Clips;
            std::unordered_map<uint64_t, uint32_t> ClipMap;
            AssetHandle TextureAtlas = 0;

            AnimationClip& Get(uint32_t p_Index)
            {
                return Clips[p_Index];
            }

            AnimationClip* Get(uint64_t p_ID)
            {
                if (ClipMap.empty())
                    BuildClipMap();

                auto it = ClipMap.find(p_ID);
                if (it == ClipMap.end())
                    return nullptr;

                return &Clips[it->second];
            }

            uint32_t GetCount() const
            {
                return (uint32_t)Clips.size();
            }

            void BuildClipMap()
            {
                if (Clips.empty()) return;

                //ClipMap.clear();
                ClipMap.reserve(Clips.size());
                for (size_t i = 0; i < Clips.size(); i++)
                {
                    Clips[i].ID          = HashString(Clips[i].Name);
                    ClipMap[Clips[i].ID] = (uint32_t)i;
                }
            }

            ASSET_CLASS_METHODS(Animation)
    };

    class KTN_API AnimationImporter
    {
        public:
            static Ref<Animation> Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
            static Ref<Animation> ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data);

            static Ref<Animation> Load(const std::string& p_Path);

            static void Save(const Ref<Animation>& p_Anim, const std::string& p_Path);

            static void SaveBin(std::ofstream& p_Out, const Ref<Animation>& p_Anim);
            static Ref<Animation> LoadBin(std::ifstream& p_In);
            static Ref<Animation> LoadBin(const Buffer& p_In);

            static void LoadBin(std::ifstream& p_In, Buffer& p_Buffer);
    };
} // namespace KTN
