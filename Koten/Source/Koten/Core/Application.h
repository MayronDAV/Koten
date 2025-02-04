#pragma once
#include "Koten/OS/Window.h"
#include "Layer.h"
#include "LayerStack.h"


namespace KTN
{
	class KTN_API Application
	{
		public:
			Application();
			~Application();

			void Run();

			void PushLayer(Layer* p_Layer);
			void PushOverlay(Layer* p_Overlay);

			void PopLayer(Layer* p_Layer);
			void PopOverlay(Layer* p_Overlay);

			void SetUpdateMinimized(bool p_Value) { m_UpdateMinimized = p_Value; }

			static Application& Get() { return *s_Instance; }

		private:
			void OnEvent(Event& p_Event);

		private:
			bool m_UpdateMinimized	= true;
			bool m_Running			= true;
			Window* m_Window		= nullptr;
			LayerStack m_LayerStack;

			static Application* s_Instance;
	};

	// defined by the client
	Application* CreateApplication(int p_Argc, char** p_Argv);

} // namespace KTN