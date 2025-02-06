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

} // namespace KTN::GLUtils