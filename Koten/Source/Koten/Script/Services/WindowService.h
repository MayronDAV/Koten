#pragma once
#include "Koten/Core/Base.h"


namespace KTN
{
    class KTN_API WindowService
    {
        public:
            virtual ~WindowService() = default;

            virtual void SetTitle(const std::string& p_Title);
            virtual void SetVsync(bool p_Value);
            virtual bool IsVsync();
            virtual bool IsMaximized();
            virtual bool IsMinimized();
            virtual uint32_t GetWidth();
            virtual uint32_t GetHeight();
            virtual void SetWidth(uint32_t p_Width);
            virtual void SetHeight(uint32_t p_Height);
            virtual void Resize(uint32_t p_Width, uint32_t p_Height);
            virtual void Close();
    };


} // namespace KTN