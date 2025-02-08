#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"


namespace KTN::GLUtils
{
	uint32_t TextureFormatToGLFormat(TextureFormat p_Format);
	uint32_t TextureFormatToGLInternalFormat(TextureFormat p_Format);
	uint32_t TextureFilterToGL(TextureFilter p_Filter, bool p_Mipmap = false);
	uint32_t TextureWrapToGL(TextureWrap p_Wrap);
	uint32_t TextureAccessToGL(TextureAccess p_Access);
	uint32_t DrawTypeToGL(DrawType p_Type);
	uint32_t CullModeToGL(CullMode p_Mode);
	uint32_t FrontFaceToGL(FrontFace p_Face);
	uint32_t PolygonModeToGL(PolygonMode p_Mode);
	uint32_t StencilOpToGL(StencilOp p_Op);
	uint32_t StencilCompareToGL(StencilCompare p_Op);
	uint32_t StencilFaceToGL(StencilFace p_Face);

} // namespace KTN::GLUtils