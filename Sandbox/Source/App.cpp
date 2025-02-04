#include "Koten/Koten.h"
#include "Koten/EntryPoint.h"




KTN::Application* KTN::CreateApplication(int p_Argc, char** p_Argv)
{
	KTN_INFO("Creating sandbox app...");

	return new KTN::Application();
}