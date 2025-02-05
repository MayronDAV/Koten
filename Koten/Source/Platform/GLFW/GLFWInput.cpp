#include "ktnpch.h"
#include "GLFWBase.h"
#include "Koten/OS/Input.h"
#include "Koten/Core/Application.h"



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
		auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();
		int mode = CursorModeToGLFW(p_Mode);
		if (mode != 0)
			glfwSetInputMode(window, GLFW_CURSOR, mode);
	}

	bool Input::IsKeyPressed(int p_Key)
	{
		auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();
		auto state = glfwGetKey(window, p_Key);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(int p_Button)
	{
		auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();
		auto state = glfwGetMouseButton(window, p_Button);
		return state == GLFW_PRESS;
	}

	glm::vec2 Input::GetMousePosition()
	{
		auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		return { (float)x, (float)y };
	}

	float Input::GetMouseX()
	{
		auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		auto window = (GLFWwindow*)Application::Get().GetWindow()->GetNative();
		return GetMousePosition().y;
	}

} // namespace KTN