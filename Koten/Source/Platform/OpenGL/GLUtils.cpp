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
				return GL_RED;
			case TextureFormat::R32_INT:
				return GL_RED_INTEGER;
			case TextureFormat::RG32_UINT:
				return GL_RG;
			case TextureFormat::RGB8:
				return GL_RGB;
			case TextureFormat::RGBA32_FLOAT:
			case TextureFormat::RGBA8:
				return GL_RGBA;

			case TextureFormat::D16:
			case TextureFormat::D32_FLOAT:
				return GL_DEPTH;

			case TextureFormat::D16_S8_UINT:
			case TextureFormat::D24_S8_UINT:
			case TextureFormat::D32_FLOAT_S8_UINT:
				return GL_DEPTH_STENCIL;
		}

		KTN_CORE_ERROR("Unsupported texture format");
		return 0;
	}

	uint32_t TextureFormatToGLInternalFormat(TextureFormat p_Format)
	{
		switch (p_Format)
		{
			case TextureFormat::R8:     			return GL_R8;
			case TextureFormat::R32_INT:			return GL_R32I;
			case TextureFormat::RG32_UINT:			return GL_RG32UI;
			case TextureFormat::RGBA32_FLOAT:		return GL_RGBA32F;
			case TextureFormat::RGB8:   			return GL_RGB8;
			case TextureFormat::RGBA8:  			return GL_RGBA8;
			case TextureFormat::D16:				return GL_DEPTH_COMPONENT16;
			case TextureFormat::D32_FLOAT:			return GL_DEPTH_COMPONENT32F;
			case TextureFormat::D32_FLOAT_S8_UINT:	return GL_DEPTH32F_STENCIL8;
			case TextureFormat::D16_S8_UINT:
			case TextureFormat::D24_S8_UINT:
				return GL_DEPTH24_STENCIL8;
		}

		KTN_CORE_ERROR("Unsupported texture format");
		return 0;
	}

	uint32_t TextureFormatToGLType(TextureFormat p_Format)
	{
		switch (p_Format)
		{
			case TextureFormat::R32_INT:			return GL_INT;
			case TextureFormat::RG32_UINT:			return GL_UNSIGNED_INT;
			case TextureFormat::D32_FLOAT:
			case TextureFormat::RGBA32_FLOAT:		
				return GL_FLOAT;
			case TextureFormat::D16:
			case TextureFormat::R8:
			case TextureFormat::RGB8:
			case TextureFormat::RGBA8:
				return GL_UNSIGNED_BYTE;
			case TextureFormat::D16_S8_UINT:
			case TextureFormat::D24_S8_UINT:
				return GL_UNSIGNED_BYTE | GL_UNSIGNED_INT;
			case TextureFormat::D32_FLOAT_S8_UINT:	
				return GL_FLOAT | GL_UNSIGNED_INT;
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

	uint32_t DrawTypeToGL(DrawType p_Type)
	{
		switch (p_Type)
		{
			case KTN::DrawType::POINTS:  	return GL_POINTS;
			case KTN::DrawType::TRIANGLES:  return GL_TRIANGLES;
			case KTN::DrawType::LINES:		return GL_LINES;
		}

		KTN_CORE_ERROR("Unknown draw type!");
		return GL_READ_WRITE;
	}

	uint32_t CullModeToGL(CullMode p_Mode)
	{
		switch (p_Mode)
		{
			case CullMode::FRONT: 			return GL_FRONT;
			case CullMode::BACK: 			return GL_BACK;
			case CullMode::FRONTANDBACK:	return GL_FRONT_AND_BACK;
		}

		KTN_CORE_ERROR("Unknown cull mode!");
		return GL_BACK;
	}

	uint32_t FrontFaceToGL(FrontFace p_Face)
	{
		switch (p_Face)
		{
			case FrontFace::CLOCKWISE: 			return GL_CW;
			case FrontFace::COUNTER_CLOCKWISE: 	return GL_CCW;
		}

		KTN_CORE_ERROR("Unknown front face!");
		return GL_CW;
	}

	uint32_t PolygonModeToGL(PolygonMode p_Mode)
	{
		switch (p_Mode)
		{
			case PolygonMode::FILL: 	return GL_FILL;
			case PolygonMode::LINE: 	return GL_LINE;
			case PolygonMode::POINT: 	return GL_POINT;
		}

		KTN_CORE_ERROR("Unknown polygon mode!");
		return GL_FILL;
	}

	uint32_t StencilOpToGL(StencilOp p_Op)
	{
		switch (p_Op)
		{
			case StencilOp::KEEP: 		return GL_KEEP;
			case StencilOp::ZERO: 		return GL_ZERO;
			case StencilOp::REPLACE: 	return GL_REPLACE;
			case StencilOp::INCR: 		return GL_INCR;
			case StencilOp::DECR: 		return GL_DECR;
			case StencilOp::INVERT: 	return GL_INVERT;
			case StencilOp::INCR_WRAP: 	return GL_INCR_WRAP;
			case StencilOp::DECR_WRAP: 	return GL_DECR_WRAP;
		}

		KTN_CORE_ERROR("Unknown stencil op!");
		return GL_KEEP;
	}

	uint32_t StencilCompareToGL(StencilCompare p_Op)
	{
		switch (p_Op)
		{
			case StencilCompare::NEVER:		return GL_NEVER;
			case StencilCompare::LESS: 		return GL_LESS;
			case StencilCompare::EQUAL: 	return GL_EQUAL;
			case StencilCompare::LEQUAL: 	return GL_LEQUAL;
			case StencilCompare::GREATER: 	return GL_GREATER;
			case StencilCompare::NOTEQUAL: 	return GL_NOTEQUAL;
			case StencilCompare::GEQUAL: 	return GL_GEQUAL;
			case StencilCompare::ALWAYS: 	return GL_ALWAYS;
		}

		KTN_CORE_ERROR("Unknown stencil compare!");
		return GL_ALWAYS;
	}

	uint32_t StencilFaceToGL(StencilFace p_Face)
	{
		switch (p_Face)
		{
			case StencilFace::FRONT: 			return GL_FRONT;
			case StencilFace::BACK: 			return GL_BACK;
			case StencilFace::FRONT_AND_BACK:	return GL_FRONT_AND_BACK;
		}

		KTN_CORE_ERROR("Unknown stencil face!");
		return GL_FRONT_AND_BACK;
	}

} // namespace KTN::GLUtils
