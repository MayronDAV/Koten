#include "ktnpch.h"
#include "GLFWBase.h"



namespace KTN
{
	double Time::GetTime()
	{
		return glfwGetTime();
	}

} // namespace KTN