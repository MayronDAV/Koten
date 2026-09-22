#include "ktnpch.h"
#include "GLFWBase.h"
#include "Koten/OS/Input.h"
#include "Koten/Core/Application.h"
#include "Koten/OS/KeyCodes.h"

#if KTN_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <X11/Xlib.h>

using X11Window = Window;
#endif



namespace KTN
{
    namespace
    {
        static int CursorModeToGLFW(CursorMode p_Mode)
        {
            switch (p_Mode)
            {
                case CursorMode::Normal:   return GLFW_CURSOR_NORMAL;
                case CursorMode::Hidden:   return GLFW_CURSOR_HIDDEN;
                case CursorMode::Disabled: return GLFW_CURSOR_DISABLED;
            }

            KTN_GLFW_ERROR("Unknown cursor mode!");
            return 0;
        }

    } // namespace

    void Input::SetCursorMode(CursorMode p_Mode)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();
        int mode = CursorModeToGLFW(p_Mode);
        if (mode != 0)
            glfwSetInputMode(window, GLFW_CURSOR, mode);
    }

    bool Input::IsKeyPressed(int p_Key)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();
        auto state = glfwGetKey(window, p_Key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsKeyJustReleased(int p_Key)
    {
        KTN_PROFILE_FUNCTION_LOW();
        auto& window = Application::Get().GetWindow();
        auto state = window->GetKeyState(p_Key);
        return state == KeyState::RELEASED;
    }

    bool Input::IsKeyJustPressed(int p_Key)
    {
        KTN_PROFILE_FUNCTION_LOW();
        auto& window = Application::Get().GetWindow();
        auto state = window->GetKeyState(p_Key);
        return state == KeyState::PRESSED;
    }

    bool Input::IsKeyJustHeld(int p_Key)
    {
        KTN_PROFILE_FUNCTION_LOW();
        auto& window = Application::Get().GetWindow();
        auto state = window->GetKeyState(p_Key);
        return state == KeyState::HELD;
    }

    bool Input::IsMouseButtonPressed(int p_Button)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();
        auto state = glfwGetMouseButton(window, p_Button);
        return state == GLFW_PRESS;
    }

    bool Input::IsMouseButtonReleased(int p_Button)
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();
        auto state = glfwGetMouseButton(window, p_Button);
        return state == GLFW_RELEASE;
    }

    glm::vec2 Input::GetCursorPosition()
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return { (float)x, (float)y };
    }

#if KTN_LINUX

    glm::vec2 Input::GetMousePosition()
    {
        KTN_PROFILE_FUNCTION_LOW();

        auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();

        Display* display = glfwGetX11Display();
        if (!display)
            return { 0.0f, 0.0f };

        X11Window root;
        X11Window child;

        int rootX = 0;
        int rootY = 0;
        int winX  = 0;
        int winY  = 0;

        unsigned int mask = 0;

        if (!XQueryPointer(
            display,
            DefaultRootWindow(display),
            &root,
            &child,
            &rootX,
            &rootY,
            &winX,
            &winY,
            &mask))
        {
            return { 0.0f, 0.0f };
        }

        return {
            static_cast<float>(rootX),
            static_cast<float>(rootY)
        };
    }
#else
#if !KTN_WINDOWS
    glm::vec2 Input::GetMousePosition()
    {
        return Input::GetCursorPosition();
    }
#endif
#endif

    int Input::GetKeyPressed()
    {
        for (int key : Key::Map)
        {
            if (IsKeyPressed(key))
                return key;
        }

        return -1;
    }

} // namespace KTN