#include "ktnpch.h"
#include "ShaderStage.h"
#include "Koten/Core/ShaderModuleLibrary.h"



namespace KTN
{
    namespace
    {
        static ShaderType ShaderTypeFromString(const std::string& p_Path)
        {
            auto ext = FileSystem::GetExtension(p_Path);

            if (ext == ".vertex" || ext == ".vert")
                return ShaderType::Vertex;
            if (ext == ".fragment" || ext == ".frag")
                return ShaderType::Fragment;
            if (ext == ".geometry" || ext == ".geom")
                return ShaderType::Geometry;
            if (ext == ".tesc")
                return ShaderType::TessControl;
            if (ext == ".tese")
                return ShaderType::TessEvaluation;

            KTN_CORE_ERROR("Unknown shader type extension!")
            return ShaderType::None;
        }

    } // namespace

    Ref<ShaderStage> ShaderStageImporter::Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::ShaderStage)
        {
            KTN_CORE_ERROR("Invalid asset type for shader stage import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        auto shaderStage      = CreateRef<ShaderStage>();
        shaderStage->Handle   = p_Handle;
        shaderStage->m_Name   = FileSystem::GetStem(p_Metadata.FilePath);
        shaderStage->m_Stage  = ShaderTypeFromString(p_Metadata.FilePath);
        shaderStage->m_Path   = p_Metadata.FilePath;
        shaderStage->m_Source = FileSystem::ReadFile(p_Metadata.FilePath);

        StageInfo info        = {};
        info.Handle           = p_Handle;
        info.Path             = p_Metadata.FilePath;
        info.Stage            = shaderStage->m_Stage;

        info.Source           = shaderStage->m_Source;

        ShaderModuleLibrary::Get()->PushStage(info);
        return shaderStage;
    }

    Ref<ShaderStage> ShaderStageImporter::ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::ShaderStage)
        {
            KTN_CORE_ERROR("Invalid asset type for shader stage import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        auto shaderStage      = CreateRef<ShaderStage>();
        shaderStage->Handle   = p_Handle;
        shaderStage->m_Name   = FileSystem::GetStem(p_Metadata.FilePath);
        shaderStage->m_Stage  = ShaderTypeFromString(p_Metadata.FilePath); // Set the appropriate shader stage based on the file extension or metadata
        shaderStage->m_Path   = p_Metadata.FilePath;
        BufferReader reader(p_Data);
        shaderStage->m_Source = Utils::ReadString(reader);

        return shaderStage;
    }

    void ShaderStageImporter::SaveBin(std::ofstream& p_Out, const Ref<ShaderStage>& p_Stage)
    {
        KTN_PROFILE_FUNCTION();

        p_Out.write(reinterpret_cast<const char*>(&p_Stage->Handle), sizeof(p_Stage->Handle));

        ShaderType type = p_Stage->GetStage();
        p_Out.write(reinterpret_cast<const char*>(&type), sizeof(type));

        Utils::WriteString(p_Out, p_Stage->GetSource());
    }

    void ShaderStageImporter::LoadBin(std::ifstream& p_In, Buffer& p_Buffer)
    {
        KTN_PROFILE_FUNCTION();

        AssetHandle handle = 0;
        p_In.read(reinterpret_cast<char*>(&handle), sizeof(handle));
        p_Buffer.Write(&handle, sizeof(handle));

        ShaderType type    = ShaderType::None;
        p_In.read(reinterpret_cast<char*>(&type), sizeof(type));
        p_Buffer.Write(&type, sizeof(type));

        std::string source = Utils::ReadString(p_In);
        Utils::WriteString(p_Buffer, source);

        StageInfo info     = {};
        info.Handle        = handle;
        info.Path          = "";
        info.Stage         = type;
        info.Source        = source;

        ShaderModuleLibrary::Get()->PushStage(info);
    }

} // namespace KTN
