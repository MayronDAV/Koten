#pragma once
#include "Koten/Core/Application.h"
#include "Koten/Core/Log.h"

// defined by the client
extern KTN::Application* KTN::CreateApplication(int p_Argc, char** p_Argv);



namespace KTN
{
	int Main(int p_Argc, char** p_Argv)
	{
	#ifndef KTN_DIST
		Log::Init();
	#endif

		auto app = CreateApplication(p_Argc, p_Argv);
		if (app) 
		{
			app->Run();
			delete app;
		}
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
			KTN_CORE_ERROR("Could not redirect stdout");
		}
		if (freopen("/dev/null", "w", stderr) == nullptr) 
		{
			KTN_CORE_ERROR("Could not redirect sterr");
		}
	#endif

		return KTN::Main(p_Argc, p_Argv);
	}
#endif