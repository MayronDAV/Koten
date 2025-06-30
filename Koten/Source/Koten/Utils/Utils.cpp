#include "ktnpch.h"
#include "Utils.h"

#include <stb/stb_image.h>
#include <stb/stb_image_resize2.h>



namespace KTN::Utils
{
	KTN_API uint32_t DataTypeSize(DataType p_Type)
	{
		switch (p_Type)
		{
			case DataType::Float:		return 4;
			case DataType::Float2:		return 4 * 2;
			case DataType::Float3:		return 4 * 3;
			case DataType::Float4:		return 4 * 4;
			case DataType::Float3x3:	return 4 * 3 * 3;
			case DataType::Float4x4:	return 4 * 4 * 4;
			case DataType::Int:			return 4;
			case DataType::Int2:		return 4 * 2;
			case DataType::Int3:		return 4 * 3;
			case DataType::Int4:		return 4 * 4;
			case DataType::Int3x3:		return 4 * 3 * 3;
			case DataType::Int4x4:		return 4 * 4 * 4;
			case DataType::Bool:		return 1;
		}

		KTN_CORE_ERROR("Unknown ShaderDataType!");
		return 0;
	}

	KTN_API uint8_t TextureFormatToChannels(TextureFormat p_Format)
	{
		switch (p_Format)
		{
			case TextureFormat::R8:
			case TextureFormat::R32_INT:
				return 1;

			case TextureFormat::RG32_UINT:
				return 2;

			case TextureFormat::RGB8:
			case TextureFormat::RGB32_FLOAT:
				return 3;

			case TextureFormat::RGBA8:
			case TextureFormat::RGBA32_FLOAT:
				return 4;
		}

		KTN_CORE_ERROR("Unknown texture format!");
		return 0;
	}

	KTN_API uint8_t TextureFormatToBytesPerChannels(TextureFormat p_Format)
	{
		switch (p_Format)
		{
			case TextureFormat::R8:
			case TextureFormat::RGB8:
			case TextureFormat::RGBA8:
				return 1;

			case TextureFormat::R32_INT:
			case TextureFormat::RG32_UINT:
			case TextureFormat::RGBA32_FLOAT:
			case TextureFormat::RGB32_FLOAT:
				return 4;
		}

		KTN_CORE_ERROR("Unknown texture format!");
		return 0;
	}

	static uint32_t s_MaxWidth = 2048;
	static uint32_t s_MaxHeight = 2048;

	KTN_API void GetMaxImageSize(uint32_t* p_Width, uint32_t* p_Height)
	{
		*p_Width = s_MaxWidth;
		*p_Height = s_MaxHeight;
	}

	KTN_API void SetMaxImageSize(uint32_t p_Width, uint32_t p_Height)
	{
		s_MaxWidth = p_Width;
		s_MaxHeight = p_Height;
	}

	KTN_API uint8_t* LoadImageFromFile(const char* p_Path, uint32_t* p_Width, uint32_t* p_Height, uint32_t* p_Channels, uint32_t* p_Bytes, bool* p_IsHDR, bool p_FlipY)
	{
		KTN_PROFILE_FUNCTION();

		stbi_set_flip_vertically_on_load(p_FlipY);

		int texWidth = 0, texHeight = 0, texChannels = 0;
		stbi_uc* pixels = nullptr;
		int sizeOfChannel = 8;
		if (stbi_is_hdr(p_Path))
		{
			sizeOfChannel = 32;
			pixels = (uint8_t*)stbi_loadf(p_Path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

			if (p_IsHDR) *p_IsHDR = true;
		}
		else
		{
			pixels = stbi_load(p_Path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

			if (p_IsHDR) *p_IsHDR = false;
		}

		if (!p_IsHDR && s_MaxWidth > 0 && s_MaxHeight > 0 && ((uint32_t)texWidth > s_MaxWidth || (uint32_t)texHeight > s_MaxHeight))
		{
			uint32_t texWidthOld = texWidth, texHeightOld = texHeight;
			float aspectRatio = static_cast<float>(texWidth) / static_cast<float>(texHeight);
			if ((uint32_t)texWidth > s_MaxWidth)
			{
				texWidth = s_MaxWidth;
				texHeight = static_cast<uint32_t>(s_MaxWidth / aspectRatio);
			}
			if ((uint32_t)texHeight > s_MaxHeight)
			{
				texHeight = s_MaxHeight;
				texWidth = static_cast<uint32_t>(s_MaxHeight * aspectRatio);
			}

			int resizedChannels = texChannels;
			uint8_t* resizedPixels = (stbi_uc*)malloc(texWidth * texHeight * resizedChannels);

			if (p_IsHDR)
			{
				stbir_resize_float_linear((float*)pixels, texWidthOld, texHeightOld, 0, (float*)resizedPixels, texWidth, texHeight, 0, STBIR_RGBA);
			}
			else
			{
				stbir_resize_uint8_linear(pixels, texWidthOld, texHeightOld, 0, resizedPixels, texWidth, texHeight, 0, STBIR_RGBA);
			}

			free(pixels);
			pixels = resizedPixels;
		}

		if (!pixels)
		{
			KTN_CORE_ERROR("Could not load image '{}'!", p_Path);
			// Return magenta checkerboad image

			texChannels = 4;

			if (p_Width)	*p_Width = 2;
			if (p_Height)	*p_Height = 2;
			if (p_Bytes)	*p_Bytes = sizeOfChannel / 8;
			if (p_Channels) *p_Channels = texChannels;

			const int32_t size = (*p_Width) * (*p_Height) * texChannels;
			uint8_t* data = new uint8_t[size];

			uint8_t datatwo[16] = {
				255, 0  , 255, 255,
				0,   0  , 0,   255,
				0,   0  , 0,   255,
				255, 0  , 255, 255
			};

			memcpy(data, datatwo, size);

			return data;
		}

		if (texChannels != 4)
			texChannels = 4;

		if (p_Width)	*p_Width = texWidth;
		if (p_Height)	*p_Height = texHeight;
		if (p_Bytes)	*p_Bytes = sizeOfChannel / 8;
		if (p_Channels) *p_Channels = texChannels;

		const uint64_t size = uint64_t(texWidth) * uint64_t(texHeight) * uint64_t(texChannels) * uint64_t(sizeOfChannel / 8U);
		uint8_t* result = new uint8_t[size];
		memcpy(result, pixels, size);

		stbi_image_free(pixels);
		return result;
	}

} // namespace KTN::Utils
