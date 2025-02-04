#include "Koten/Koten.h"
#include "Koten/EntryPoint.h"




KTN::Application* KTN::CreateApplication(int p_Argc, char** p_Argv)
{
	return new KTN::Application();
}