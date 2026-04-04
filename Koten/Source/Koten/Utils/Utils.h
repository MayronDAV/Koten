#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"
#include "Koten/OS/FileDialog.h"

// lib
#include <vector>



namespace KTN::Utils
{
    KTN_API uint32_t DataTypeSize(DataType p_Type);
    KTN_API uint8_t  TextureFormatToChannels(TextureFormat p_Format);
    KTN_API uint8_t  TextureFormatToBytesPerChannels(TextureFormat p_Format);

    KTN_API void GetMaxImageSize(uint32_t* p_Width, uint32_t* p_Height);
    KTN_API void SetMaxImageSize(uint32_t p_Width, uint32_t p_Height);
    KTN_API uint8_t* LoadImageFromFile(const char* p_Path, uint32_t* p_Width, uint32_t* p_Height, uint32_t* p_Channels, uint32_t* p_Bytes, bool* p_IsHDR = nullptr, bool p_FlipY = true);

    KTN_API void WriteString(std::ofstream& p_Out, const std::string& p_Str);
    KTN_API void WriteString(Buffer& p_Buffer, const std::string& p_String);
    KTN_API std::string ReadString(BufferReader& p_Buffer);
    KTN_API std::string ReadString(std::ifstream& p_In);

    KTN_API std::vector<char> BuildFilter(const FilterList& p_Filters);

} // namespace KTN::Utils