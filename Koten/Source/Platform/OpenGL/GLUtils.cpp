#include "ktnpch.h"
#include "GLBase.h"
#include "GLUtils.h"



namespace KTN::GLUtils
{
	uint32_t TextureFormatToGLFormat(TextureFormat p_Format)
	{
		switch (p_Format)
		{
			case TextureFormat::R8:
			case TextureFormat::R32_INT:
				return GL_RED;
			case TextureFormat::RG32_UINT:
				return GL_RG;
			case TextureFormat::RGB8:
				return GL_RGB;
			case TextureFormat::RGBA32_FLOAT:
			case TextureFormat::RGBA8:
				return GL_RGBA;
		}

		KTN_CORE_ERROR("Unsupported texture format");
		return 0;
	}

	uint32_t TextureFormatToGLInternalFormat(TextureFormat p_Format)
	{
		switch (p_Format)
		{
			case TextureFormat::R8:     		return GL_R8;
			case TextureFormat::R32_INT:		return GL_R32I;
			case TextureFormat::RG32_UINT:		return GL_RG32UI;
			case TextureFormat::RGBA32_FLOAT:	return GL_RGBA32F;
			case TextureFormat::RGB8:   		return GL_RGB8;
			case TextureFormat::RGBA8:  		return GL_RGBA8;
		}

		KTN_CORE_ERROR("Unsupported texture format");
		return 0;
	}

	uint32_t TextureFilterToGL(TextureFilter p_Filter, bool p_Mipmap)
	{
		switch (p_Filter)
		{
			case TextureFilter::LINEAR: return p_Mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
			case TextureFilter::NEAREST: return p_Mipmap ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST;
		}

		KTN_CORE_ERROR("Unsupported texture filter");
		return 0;
	}

	uint32_t TextureWrapToGL(TextureWrap p_Wrap)
	{
		switch (p_Wrap)
		{
			case TextureWrap::REPEAT:           return GL_REPEAT;
			case TextureWrap::MIRRORED_REPEAT:  return GL_MIRRORED_REPEAT;
			case TextureWrap::CLAMP_TO_BORDER:  return GL_CLAMP_TO_BORDER;
			case TextureWrap::CLAMP_TO_EDGE:    return GL_CLAMP_TO_EDGE;
		}

		KTN_CORE_ERROR("Unsupported texture wrap");
		return 0;
	}

	uint32_t TextureAccessToGL(TextureAccess p_Access)
	{
		switch (p_Access)
		{
			case TextureAccess::READ_ONLY:  return GL_READ_ONLY;
			case TextureAccess::WRITE_ONLY: return GL_WRITE_ONLY;
			case TextureAccess::READ_WRITE: return GL_READ_WRITE;
		}

		KTN_CORE_ERROR("Unknown texture access!");
		return GL_READ_WRITE;
	}

} // namespace KTN::GLUtils
