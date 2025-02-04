#pragma once


namespace KTN
{
	class KTN_API Application
	{
		public:
			Application();
			~Application();

			void Run();
	};

	// defined by the client
	Application* CreateApplication(int p_Argc, char** p_Argv);

} // namespace KTN