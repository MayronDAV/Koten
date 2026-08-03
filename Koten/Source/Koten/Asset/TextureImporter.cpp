#include "ktnpch.h"
#include "TextureImporter.h"



namespace KTN
{
    Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::Texture2D)
        {
            KTN_CORE_ERROR("Invalid asset type import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        Ref<Texture2D> texture       = nullptr;
        if (p_Metadata.AssetData)
            texture                  = LoadTexture2D(p_Metadata.FilePath, *static_cast<TextureSpecification*>(p_Metadata.AssetData));
        else
            texture                  = LoadTexture2D(p_Metadata.FilePath);

        if (texture) texture->Handle = p_Handle;

        return texture;
    }

    Ref<Texture2D> TextureImporter::ImportTexture2DFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Metadata.Type != AssetType::Texture2D)
        {
            KTN_CORE_ERROR("Invalid asset type import: {}", GetAssetTypeName(p_Metadata.Type));
            return nullptr;
        }

        auto spec           = p_Metadata.AssetData ? *static_cast<TextureSpecification*>(p_Metadata.AssetData) : TextureSpecification{};

        BufferReader reader(p_Data);

        bool renderTarget   = false;
        reader.ReadBytes(&renderTarget, sizeof(renderTarget));

        bool hasData        = false;
        reader.ReadBytes(&hasData, sizeof(hasData));

        if (renderTarget)
        {
            spec.Usage      = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
            auto texture    = Texture2D::Create(spec);
            texture->Handle = p_Handle;

            return texture;
        }

        size_t dataSize     = 0;
        reader.ReadBytes(&dataSize, sizeof(dataSize));

        std::vector<uint8_t> textureData(dataSize);
        reader.ReadBytes(textureData.data(), dataSize);


        auto texture        = Texture2D::Create(spec, textureData.data(), textureData.size());
        texture->Handle     = p_Handle;

        return texture;
    }

    Ref<Texture2D> TextureImporter::LoadTexture2D(const std::string& p_Path)
    {
        KTN_PROFILE_FUNCTION();

        TextureSpecification spec = {};
        spec.WrapU                = TextureWrap::REPEAT;
        spec.WrapV                = TextureWrap::REPEAT;
        spec.MinFilter            = TextureFilter::LINEAR;
        spec.MagFilter            = TextureFilter::LINEAR;
        spec.AnisotropyEnable     = true;
        spec.GenerateMips         = true;
        spec.SRGB                 = true;
        spec.DebugName            = FileSystem::GetName(p_Path);

        return LoadTexture2D(p_Path, spec);
    }

    Ref<Texture2D> TextureImporter::LoadTexture2D(const std::string& p_Path, const TextureSpecification& p_Spec)
    {
        KTN_PROFILE_FUNCTION();

        if (FileSystem::GetExtension(p_Path) == ".ktrt")
            return LoadRenderTarget(p_Path, p_Spec);

        uint32_t width, height, channels = 4, bytes = 1;
        bool isHDR                = false;
        uint8_t* data             = Utils::LoadImageFromFile(p_Path.c_str(), &width, &height, &channels, &bytes, &isHDR, Engine::Get().GetAPI() == RenderAPI::OpenGL);

        TextureSpecification spec = p_Spec;
        spec.Width                = width;
        spec.Height               = height;
        spec.Format               = (isHDR) ? TextureFormat::RGBA32_FLOAT : TextureFormat::RGBA8;
        spec.Usage                = TextureUsage::TEXTURE_SAMPLED;

        uint64_t imageSize = uint64_t(width) * uint64_t(height) * uint64_t(channels) * uint64_t(bytes);
        auto texture              = Texture2D::Create(spec, data, imageSize);
        if (!texture)
        {
            KTN_CORE_ERROR("Failed to create texture!");
            free(data);
            return nullptr;
        }

        free(data);
        return texture;
    }

    struct RenderTargetHeader
    {
        char Magic[4]            = { 'K', 'T', 'R', 'T' };
        uint32_t Version         = 1;
    };

    void TextureImporter::CreateRenderTarget(const std::string& p_Folder, const TextureSpecification& p_Spec)
    {
        KTN_PROFILE_FUNCTION();

        FileSystem::CreateDirectories(p_Folder);
        const auto path           = std::filesystem::path(p_Folder) / "NewRenderTarget.ktrt";

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out)
        {
            KTN_CORE_ERROR("Failed to create render target file: {}", path.string());
            return;
        }

        TextureSpecification spec = p_Spec;
        spec.Usage                = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
        spec.Width                = spec.Width != 1 ? spec.Width : 800;
        spec.Height               = spec.Height != 1 ? spec.Height : 600;

        RenderTargetHeader header;
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));

        AssetHandle handle;
        out.write(reinterpret_cast<const char*>(&handle), sizeof(handle));
        out.write(reinterpret_cast<const char*>(&spec.Usage), sizeof(spec.Usage));
        out.write(reinterpret_cast<const char*>(&spec.Width), sizeof(spec.Width));
        out.write(reinterpret_cast<const char*>(&spec.Height), sizeof(spec.Height));
    }

    Ref<Texture2D> TextureImporter::LoadRenderTarget(const std::string& p_Path, const TextureSpecification& p_Spec)
    {
        KTN_PROFILE_FUNCTION();

        std::ifstream in(p_Path, std::ios::binary);
        if (!in)
        {
            KTN_CORE_ERROR("Failed to open render target file: {}", p_Path);
            return nullptr;
        }

        RenderTargetHeader header;
        in.read(reinterpret_cast<char*>(&header), sizeof(header));

        if (memcmp(header.Magic, "KTRT", 4) != 0 || header.Version != 1)
        {
            KTN_CORE_ERROR("Invalid render target format");
            return nullptr;
        }

        AssetHandle handle;
        in.read(reinterpret_cast<char*>(&handle), sizeof(handle));

        TextureSpecification spec = p_Spec;
        in.read(reinterpret_cast<char*>(&spec.Usage), sizeof(spec.Usage));
        in.read(reinterpret_cast<char*>(&spec.Width), sizeof(spec.Width));
        in.read(reinterpret_cast<char*>(&spec.Height), sizeof(spec.Height));
        spec.DebugName            = "RenderTarget_" + std::to_string(handle);

        auto texture              = Texture2D::Create(spec);
        texture->Handle           = handle;
        return texture;
    }

} // namespace KTN
