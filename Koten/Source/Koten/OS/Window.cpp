#include "ktnpch.h"
#include "Window.h"

#include "Platform/GLFW/GLFWWindow.h"



namespace KTN
{
	Unique<Window> Window::Create(const WindowSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION();

		return CreateUnique<GLFWWindow>(p_Spec);
	}

} // namespace KTN
