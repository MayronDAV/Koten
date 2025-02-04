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

			void PushLayer(const Ref<Layer>& p_Layer);
			void PushOverlay(const Ref<Layer>& p_Overlay);

			void PopLayer(const Ref<Layer>& p_Layer);
			void PopOverlay(const Ref<Layer>& p_Overlay);

			void SetUpdateMinimized(bool p_Value) { m_UpdateMinimized = p_Value; }

			static Application& Get() { return *s_Instance; }

		private:
			void OnEvent(Event& p_Event);

		private:
			bool m_UpdateMinimized	= true;
			bool m_Running			= true;
			Unique<Window> m_Window = nullptr;
			LayerStack m_LayerStack;

			static Application* s_Instance;
	};

	// defined by the client
	Application* CreateApplication(int p_Argc, char** p_Argv);

} // namespace KTN