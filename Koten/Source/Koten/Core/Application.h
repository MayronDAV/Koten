#pragma once
#include "Koten/OS/Window.h"


namespace KTN
{
	class KTN_API Application
	{
		public:
			Application();
			~Application();

			void Run();

		private:
			bool m_Running = true;
			Window* m_Window = nullptr;
	};

	// defined by the client
	Application* CreateApplication(int p_Argc, char** p_Argv);

} // namespace KTN