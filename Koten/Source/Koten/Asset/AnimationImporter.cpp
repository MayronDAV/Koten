#include "ktnpch.h"
#include "AnimationImporter.h"

// lib
#include <yaml-cpp/yaml.h>



namespace KTN
{
    Ref<Animation> AnimationImporter::Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::Animation)
        {
            KTN_CORE_ERROR("Invalid asset type for animation import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        auto anim = Load(p_Metadata.FilePath);

        return anim;
    }

    Ref<Animation> AnimationImporter::ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::Animation)
        {
            KTN_CORE_ERROR("Invalid asset type for animation import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        auto anim = LoadBin(p_Data);

        return anim;
    }

    Ref<Animation> AnimationImporter::Load(const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        if (FileSystem::GetExtension(p_Path) != ".ktanim")
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
            KTN_CORE_ERROR("Failed to load .ktanim file '{}'\n     {}", p_Path, e.what());
            return nullptr;
        }

        if (!data["AssetHandle"] || !data["TextureAtlas"])
        {
            KTN_CORE_ERROR("Failed to load file '{}'\n     Invalid animation format!", p_Path);
            return nullptr;
        }

        auto anim                  = CreateRef<Animation>();
        anim->Handle               = data["AssetHandle"].as<uint64_t>();
        anim->TextureAtlas         = data["TextureAtlas"].as<uint64_t>();

        const auto& clips          = data["Clips"];
        if (clips)
        {
            anim->Clips.reserve(clips.size());
            for (const auto& clipData : clips)
            {
                AnimationClip clip = {};
                clip.ID            = clipData["ID"].as<uint64_t>();
                clip.Name          = clipData["Name"].as<std::string>();
                clip.Mode          = (AnimationMode)clipData["Mode"].as<uint32_t>();
                clip.Speed         = clipData["Speed"].as<float>();
                clip.TotalDuration = clipData["TotalDuration"].as<float>();
                const auto& frames = clipData["Frames"];
                if (frames)
                {
                    clip.Frames.reserve(frames.size());
                    for (const auto& frameData : frames)
                    {
                        AnimationFrame frame = {};
                        frame.AtlasRegionID  = frameData["AtlasRegionID"].as<uint64_t>();
                        frame.Time           = frameData["Time"].as<float>();
                        clip.Frames.push_back(frame);
                    }
                }
                anim->Clips.push_back(clip);
            }
            anim->BuildClipMap();
        }

        return anim;
    }

    void AnimationImporter::Save(const Ref<Animation>& p_Anim, const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "AssetHandle" << YAML::Value << p_Anim->Handle;
        out << YAML::Key << "TextureAtlas" << YAML::Value << p_Anim->TextureAtlas;
        out << YAML::Key << "Clips" << YAML::Value << YAML::BeginSeq;
        for (auto& clip : p_Anim->Clips)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "ID" << YAML::Value << clip.ID;
            out << YAML::Key << "Name" << YAML::Value << clip.Name;
            out << YAML::Key << "Mode" << YAML::Value << (uint32_t)clip.Mode;
            out << YAML::Key << "Speed" << YAML::Value << clip.Speed;
            out << YAML::Key << "TotalDuration" << YAML::Value << clip.TotalDuration;
            out << YAML::Key << "Frames" << YAML::Value << YAML::BeginSeq;
            for (auto& frame : clip.Frames)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "AtlasRegionID" << YAML::Value << frame.AtlasRegionID;
                out << YAML::Key << "Time" << YAML::Value << frame.Time;
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(p_Path);
        fout << out.c_str();

        KTN_CORE_INFO("Animation serialized to path: {}", p_Path);
    }

    void AnimationImporter::SaveBin(std::ofstream& p_Out, const Ref<Animation>& p_Anim)
    {
        KTN_PROFILE_FUNCTION();

        p_Out.write(reinterpret_cast<const char*>(&p_Anim->Handle), sizeof(AssetHandle));
        p_Out.write(reinterpret_cast<const char*>(&p_Anim->TextureAtlas), sizeof(AssetHandle));

        auto size = p_Anim->Clips.size();
        p_Out.write(reinterpret_cast<const char*>(&size), sizeof(size));

        for (auto& clip : p_Anim->Clips)
        {
            p_Out.write(reinterpret_cast<const char*>(&clip.ID), sizeof(clip.ID));
            Utils::WriteString(p_Out, clip.Name);
            p_Out.write(reinterpret_cast<const char*>(&clip.Mode), sizeof(clip.Mode));
            p_Out.write(reinterpret_cast<const char*>(&clip.Speed), sizeof(clip.Speed));
            p_Out.write(reinterpret_cast<const char*>(&clip.TotalDuration), sizeof(clip.TotalDuration));

            auto frameCount = clip.Frames.size();
            p_Out.write(reinterpret_cast<const char*>(&frameCount), sizeof(frameCount));
            for (auto& frame : clip.Frames)
            {
                p_Out.write(reinterpret_cast<const char*>(&frame.AtlasRegionID), sizeof(frame.AtlasRegionID));
                p_Out.write(reinterpret_cast<const char*>(&frame.Time), sizeof(frame.Time));
            }
        }
    }

    Ref<Animation> AnimationImporter::LoadBin(std::ifstream& p_In)
    {
        KTN_PROFILE_FUNCTION();

        auto anim = CreateRef<Animation>();

        p_In.read(reinterpret_cast<char*>(&anim->Handle), sizeof(AssetHandle));
        p_In.read(reinterpret_cast<char*>(&anim->TextureAtlas), sizeof(AssetHandle));

        size_t size = 0;
        p_In.read(reinterpret_cast<char*>(&size), sizeof(size));
        anim->Clips.reserve(size);

        for (size_t i = 0; i < size; i++)
        {
            AnimationClip clip = {};
            p_In.read(reinterpret_cast<char*>(&clip.ID), sizeof(clip.ID));
            clip.Name = Utils::ReadString(p_In);
            p_In.read(reinterpret_cast<char*>(&clip.Mode), sizeof(clip.Mode));
            p_In.read(reinterpret_cast<char*>(&clip.Speed), sizeof(clip.Speed));
            p_In.read(reinterpret_cast<char*>(&clip.TotalDuration), sizeof(clip.TotalDuration));

            size_t frameCount = 0;
            p_In.read(reinterpret_cast<char*>(&frameCount), sizeof(frameCount));

            clip.Frames.reserve(frameCount);
            for (size_t j = 0; j < frameCount; j++)
            {
                AnimationFrame frame = {};
                p_In.read(reinterpret_cast<char*>(&frame.AtlasRegionID), sizeof(frame.AtlasRegionID));
                p_In.read(reinterpret_cast<char*>(&frame.Time), sizeof(frame.Time));
                clip.Frames.push_back(frame);
            }

            anim->Clips.push_back(clip);
        }

        return anim;
    }

    Ref<Animation> AnimationImporter::LoadBin(const Buffer& p_In)
    {
        KTN_PROFILE_FUNCTION();

        auto anim   = CreateRef<Animation>();
        BufferReader reader(p_In);

        reader.ReadBytes(&anim->Handle, sizeof(AssetHandle));
        reader.ReadBytes(&anim->TextureAtlas, sizeof(AssetHandle));

        size_t size = 0;
        reader.ReadBytes(&size, sizeof(size));
        anim->Clips.reserve(size);

        for (size_t i = 0; i < size; i++)
        {
            AnimationClip clip = {};
            reader.ReadBytes(&clip.ID, sizeof(clip.ID));
            clip.Name = Utils::ReadString(reader);
            reader.ReadBytes(&clip.Mode, sizeof(clip.Mode));
            reader.ReadBytes(&clip.Speed, sizeof(clip.Speed));
            reader.ReadBytes(&clip.TotalDuration, sizeof(clip.TotalDuration));

            size_t frameCount = 0;
            reader.ReadBytes(&frameCount, sizeof(frameCount));

            clip.Frames.reserve(frameCount);
            for (size_t j = 0; j < frameCount; j++)
            {
                AnimationFrame frame = {};
                reader.ReadBytes(&frame.AtlasRegionID, sizeof(frame.AtlasRegionID));
                reader.ReadBytes(&frame.Time, sizeof(frame.Time));
                clip.Frames.push_back(frame);
            }

            anim->Clips.push_back(clip);
        }

        return anim;
    }

    void AnimationImporter::LoadBin(std::ifstream& p_In, Buffer& p_Buffer)
    {
        KTN_PROFILE_FUNCTION();

        AssetHandle handle;
        p_In.read(reinterpret_cast<char*>(&handle), sizeof(AssetHandle));
        p_Buffer.Write(&handle, sizeof(AssetHandle));

        AssetHandle textureAtlas;
        p_In.read(reinterpret_cast<char*>(&textureAtlas), sizeof(AssetHandle));
        p_Buffer.Write(&textureAtlas, sizeof(AssetHandle));

        size_t size = 0;
        p_In.read(reinterpret_cast<char*>(&size), sizeof(size));
        p_Buffer.Write(&size, sizeof(size));

        for (size_t i = 0; i < size; i++)
        {
            AnimationClip clip = {};
            p_In.read(reinterpret_cast<char*>(&clip.ID), sizeof(clip.ID));
            p_Buffer.Write(&clip.ID, sizeof(clip.ID));

            clip.Name = Utils::ReadString(p_In);
            Utils::WriteString(p_Buffer, clip.Name);

            p_In.read(reinterpret_cast<char*>(&clip.Mode), sizeof(clip.Mode));
            p_Buffer.Write(&clip.Mode, sizeof(clip.Mode));

            p_In.read(reinterpret_cast<char*>(&clip.Speed), sizeof(clip.Speed));
            p_Buffer.Write(&clip.Speed, sizeof(clip.Speed));

            p_In.read(reinterpret_cast<char*>(&clip.TotalDuration), sizeof(clip.TotalDuration));
            p_Buffer.Write(&clip.TotalDuration, sizeof(clip.TotalDuration));

            size_t frameCount = 0;
            p_In.read(reinterpret_cast<char*>(&frameCount), sizeof(frameCount));
            p_Buffer.Write(&frameCount, sizeof(frameCount));

            for (size_t j = 0; j < frameCount; j++)
            {
                AnimationFrame frame = {};
                p_In.read(reinterpret_cast<char*>(&frame.AtlasRegionID), sizeof(frame.AtlasRegionID));
                p_Buffer.Write(&frame.AtlasRegionID, sizeof(frame.AtlasRegionID));

                p_In.read(reinterpret_cast<char*>(&frame.Time), sizeof(frame.Time));
                p_Buffer.Write(&frame.Time, sizeof(frame.Time));
            }
        }
    }

} // namespace KTN
