#include "ktnpch.h"
#include "Material.h"
#include "Koten/Project/Project.h"
#include "Texture.h"

// lib
#include <yaml-cpp/yaml.h>



namespace KTN
{
    void Material::Serialize(const std::string& p_Path) const
    {
        KTN_PROFILE_FUNCTION();

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Material" << YAML::Value << Handle;
        out << YAML::Key << "Name" << YAML::Value << Name;
        out << YAML::Key << "Color" << YAML::Value << AlbedoColor;
        out << YAML::Key << "Texture" << YAML::Value << Texture;
        out << YAML::EndMap;

        std::ofstream fout(p_Path);
        fout << out.c_str();

        KTN_CORE_INFO("Material serialized to path: {}", p_Path);
    }

    void Material::SerializeBin(std::ofstream& p_Out) const
    {
        KTN_PROFILE_FUNCTION();

        Utils::WriteString(p_Out, Name);
        p_Out.write(reinterpret_cast<const char*>(&AlbedoColor), sizeof(AlbedoColor));
        p_Out.write(reinterpret_cast<const char*>(&Texture), sizeof(Texture));
    }

    void Material::DeserializeBin(std::ifstream& p_In)
    {
        KTN_PROFILE_FUNCTION();

        Name = Utils::ReadString(p_In);
        p_In.read(reinterpret_cast<char*>(&AlbedoColor), sizeof(AlbedoColor));
        p_In.read(reinterpret_cast<char*>(&Texture), sizeof(Texture));
    }

    void Material::DeserializeBin(BufferReader& p_In)
    {
        KTN_PROFILE_FUNCTION();

        Name = Utils::ReadString(p_In);
        p_In.ReadBytes(&AlbedoColor, sizeof(AlbedoColor));
        p_In.ReadBytes(&Texture, sizeof(Texture));
    }

    void Material::DeserializeBin(std::ifstream& p_In, Buffer& p_Buffer)
    {
        KTN_PROFILE_FUNCTION();

        std::string name = Utils::ReadString(p_In);
        glm::vec4 albedoColor{};
        AssetHandle texture = 0;
        p_In.read(reinterpret_cast<char*>(&albedoColor), sizeof(albedoColor));
        p_In.read(reinterpret_cast<char*>(&texture), sizeof(texture));

        Utils::WriteString(p_Buffer, name);
        p_Buffer.Write(&albedoColor, sizeof(albedoColor));
        p_Buffer.Write(&texture, sizeof(texture));
    }

    AssetHandle Material::GetDefault()
    {
        KTN_PROFILE_FUNCTION();

        static auto path           = (Project::GetAssetFileSystemPath("Materials") / "Default.ktmat").string();
        FileSystem::CreateDirectories(Project::GetAssetFileSystemPath("Materials").string());
        if (!AssetManager::Get()->HasAsset(AssetType::Material, path))
        {
            if (FileSystem::Exists(path))
            {
                auto material      = AssetManager::Get()->ImportAsset(AssetType::Material, path);
                if (!material)
                {
                    KTN_CORE_ERROR("Failed to import default Material: {}", path);
                    return (AssetHandle)0;
                }
                return material;
            }

            auto material          = CreateRef<Material>();
            material->Name         = "Default";
            material->Serialize(path);

            AssetHandle handle     = {};
            AssetMetadata metadata = {};
            metadata.FilePath      = path;
            metadata.Type          = AssetType::Material;
            auto success           = AssetManager::Get()->ImportAsset(handle, metadata, material);
            if (!success)
            {
                KTN_CORE_ERROR("Failed to import default Material: {}", path);
                return (AssetHandle)0;
            }

            return handle;
        }

        return AssetManager::Get()->GetHandleByPath(path);
    }

} // namespace KTN
