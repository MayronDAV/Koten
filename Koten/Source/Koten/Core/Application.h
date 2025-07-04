#pragma once
#include "Koten/OS/Window.h"
#include "Layer.h"
#include "LayerStack.h"
#include "Koten/ImGui/ImGuiLayer.h"


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

			Unique<Window>& GetWindow() { return m_Window; }
			void Close() { m_Running = false; }
			const Ref<ImGuiLayer>& GetImGui() { return m_ImGui; }
			LayerStack& GetLayerStack() { return m_LayerStack; }

			static Application& Get() { return *s_Instance; }

			void SubmitToMainThread(const std::function<void()>& p_Func);

		private:
			void OnEvent(Event& p_Event);

			void ExecuteMainThreadQueue();

		private:
			bool m_UpdateMinimized	= true;
			bool m_Running			= true;
			Unique<Window> m_Window = nullptr;
			LayerStack m_LayerStack;

			Ref<ImGuiLayer> m_ImGui = nullptr;

			double m_LastTime		= 0.0;
			double m_Counter		= 0.0;

			std::vector<std::function<void()>> m_MainThreadQueue;
			std::mutex m_MainThreadQueueMutex;

			static Application* s_Instance;
	};

	// defined by the client
	Application* CreateApplication(int p_Argc, char** p_Argv);

} // namespace KTN