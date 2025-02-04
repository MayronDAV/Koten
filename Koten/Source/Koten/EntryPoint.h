#pragma once
#include "Koten/Core/Application.h"

// defined by the client
extern KTN::Application* KTN::CreateApplication(int p_Argc, char** p_Argv);



namespace KTN
{
	int Main(int p_Argc, char** p_Argv)
	{
		auto app = KTN::CreateApplication(p_Argc, p_Argv);
		if (!app) 
		{
			// TODO: Log
			return -1;
		}
		app->Run();
		delete app;
		return 0;
	}

} // namespace KTN


#if defined(KTN_WINDOWS) && defined(KTN_DIST)
	#include <Windows.h>

	int APIENTRY WinMain(HINSTANCE p_hInst, HINSTANCE p_hInstPrev, PSTR p_cmdline, int p_cmdshow)
	{
		return KTN::Main(__argc, __argv);
	}
#else
	#if defined(KTN_DIST) && defined(KTN_LINUX)
		#include <cstdio>
	#endif

	int main(int p_Argc, char** p_Argv)
	{
	#if defined(KTN_DIST) && defined(KTN_LINUX)
		if (freopen("/dev/null", "w", stdout) == nullptr) 
		{
			// TODO: Log
		}
		if (freopen("/dev/null", "w", stderr) == nullptr) 
		{
			// TODO: Log
		}
	#endif

		return KTN::Main(p_Argc, p_Argv);
	}
#endif