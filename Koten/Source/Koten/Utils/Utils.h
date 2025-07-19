#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"



namespace KTN::Utils
{
	KTN_API uint32_t DataTypeSize(DataType p_Type);
	KTN_API uint8_t  TextureFormatToChannels(TextureFormat p_Format);
	KTN_API uint8_t  TextureFormatToBytesPerChannels(TextureFormat p_Format);

	KTN_API void GetMaxImageSize(uint32_t* p_Width, uint32_t* p_Height);
	KTN_API void SetMaxImageSize(uint32_t p_Width, uint32_t p_Height);
	KTN_API uint8_t* LoadImageFromFile(const char* p_Path, uint32_t* p_Width, uint32_t* p_Height, uint32_t* p_Channels, uint32_t* p_Bytes, bool* p_IsHDR = nullptr, bool p_FlipY = true);

	KTN_API void WriteString(std::ofstream& p_Out, const std::string& p_Str);
	KTN_API std::string ReadString(std::ifstream& p_In);

} // namespace KTN::Utils