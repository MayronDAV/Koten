#include "ktnpch.h"
#include "TextureImporter.h"



namespace KTN
{
	Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle p_Handle, const AssetMetadata& p_Metadata)
	{
		KTN_PROFILE_FUNCTION();

		if (p_Metadata.Type != AssetType::Texture2D)
		{
			KTN_CORE_ERROR("Invalid asset type for texture2D import: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		Ref<Texture2D> texture = nullptr;
		if (p_Metadata.AssetData)
			texture = LoadTexture2D(p_Metadata.FilePath, *static_cast<TextureSpecification*>(p_Metadata.AssetData));
		else
			texture = LoadTexture2D(p_Metadata.FilePath);

		if (texture) texture->Handle = p_Handle;

		return texture;
	}

	Ref<Texture2D> TextureImporter::ImportTexture2DFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data)
	{
		KTN_PROFILE_FUNCTION();

		if (p_Metadata.Type != AssetType::Texture2D)
		{
			KTN_CORE_ERROR("Invalid asset type for texture2D import: {}", GetAssetTypeName(p_Metadata.Type));
			return nullptr;
		}

		BufferReader reader(p_Data);

		size_t dataSize = 0;
		reader.ReadBytes(&dataSize, sizeof(dataSize));

		std::vector<uint8_t> textureData(dataSize);
		reader.ReadBytes(textureData.data(), dataSize);

		auto spec = p_Metadata.AssetData ? *static_cast<TextureSpecification*>(p_Metadata.AssetData) : TextureSpecification{};

		auto texture = Texture2D::Create(spec, textureData.data(), textureData.size());
		texture->Handle = p_Handle;

		return texture;
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		TextureSpecification spec	= {};
		spec.WrapU					= TextureWrap::REPEAT;
		spec.WrapV					= TextureWrap::REPEAT;
		spec.MinFilter				= TextureFilter::LINEAR;
		spec.MagFilter				= TextureFilter::LINEAR;
		spec.AnisotropyEnable		= true;
		spec.GenerateMips			= true;
		spec.SRGB					= true;
		spec.DebugName				= FileSystem::GetName(p_Path);

		return LoadTexture2D(p_Path, spec);
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const std::string& p_Path, const TextureSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION();

		uint32_t width, height, channels = 4, bytes = 1;
		bool isHDR					= false;
		uint8_t* data				= Utils::LoadImageFromFile(p_Path.c_str(), &width, &height, &channels, &bytes, &isHDR, Engine::Get().GetAPI() == RenderAPI::OpenGL);

		TextureSpecification spec	= p_Spec;
		spec.Width					= width;
		spec.Height					= height;
		spec.Format					= (isHDR) ? TextureFormat::RGBA32_FLOAT : TextureFormat::RGBA8;
		spec.Usage					= TextureUsage::TEXTURE_SAMPLED;

		uint64_t imageSize = uint64_t(width) * uint64_t(height) * uint64_t(channels) * uint64_t(bytes);
		auto texture				= Texture2D::Create(spec, data, imageSize);
		if (!texture)
		{
			KTN_CORE_ERROR("Failed to create texture!");
			free(data);
			return nullptr;
		}

		free(data);
		return texture;
	}

} // namespace KTN
