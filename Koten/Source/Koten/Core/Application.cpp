#include "Application.h"
#include "Log.h"


namespace KTN
{
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		int count = 0;
		while (true)
		{
			KTN_CORE_INFO("Frame: {}", count);
			count++;
		}
	}

} // namespace KTN
