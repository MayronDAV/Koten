#include "ktnpch.h"
#include "WindowService.h"
#include "Koten/Core/Application.h"


namespace KTN
{
    void WindowService::Close()
    {
        KTN_PROFILE_FUNCTION_LOW();

        Application::Get().GetWindow()->Close();
        Application::Get().Close();
    }

    void WindowService::SetTitle(const std::string& p_Title)
    {
        KTN_PROFILE_FUNCTION_LOW();

        Application::Get().GetWindow()->SetTitle(p_Title);
    }

    void WindowService::SetVsync(bool p_Value)
    {
        KTN_PROFILE_FUNCTION_LOW();

        Application::Get().GetWindow()->SetVsync(p_Value);
    }

    bool WindowService::IsVsync()
    {
        KTN_PROFILE_FUNCTION_LOW();

        return Application::Get().GetWindow()->IsVsync();
    }

    bool WindowService::IsMaximized()
    {
        KTN_PROFILE_FUNCTION_LOW();

        return Application::Get().GetWindow()->IsMaximized();
    }

    bool WindowService::IsMinimized()
    {
        KTN_PROFILE_FUNCTION_LOW();

        return Application::Get().GetWindow()->IsMinimized();
    }

    uint32_t WindowService::GetWidth()
    {
        KTN_PROFILE_FUNCTION_LOW();

        return Application::Get().GetWindow()->GetWidth();
    }

    uint32_t WindowService::GetHeight()
    {
        KTN_PROFILE_FUNCTION_LOW();

        return Application::Get().GetWindow()->GetHeight();
    }

    void WindowService::SetWidth(uint32_t p_Width)
    {
        KTN_PROFILE_FUNCTION_LOW();

        Application::Get().GetWindow()->Resize(p_Width, Application::Get().GetWindow()->GetHeight());
    }

    void WindowService::SetHeight(uint32_t p_Height)
    {
        KTN_PROFILE_FUNCTION_LOW();

        Application::Get().GetWindow()->Resize(Application::Get().GetWindow()->GetWidth(), p_Height);
    }

    void WindowService::Resize(uint32_t p_Width, uint32_t p_Height)
    {
        KTN_PROFILE_FUNCTION_LOW();

        Application::Get().GetWindow()->Resize(p_Width, p_Height);
    }

} // namespace KTN
