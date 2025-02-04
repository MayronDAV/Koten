#include "ktnpch.h"
#include "Window.h"

#include "Platform/GLFW/GLFWWindow.h"



namespace KTN
{
	Window* Window::Create(const WindowSpecification& p_Spec)
	{
		return new GLFWWindow(p_Spec);
	}

} // namespace KTN
