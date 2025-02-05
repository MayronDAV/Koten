#include "Koten/Koten.h"
#include "Koten/EntryPoint.h"


namespace KTN
{
	class SandboxLayer : public Layer
	{
		public:
			SandboxLayer() : Layer("SandboxLayer") {}
			~SandboxLayer() = default;

			void OnAttach() override { KTN_INFO("Attaching..."); }
			void OnDetach() override { KTN_INFO("Detaching..."); }
			void OnUpdate() override {}
			void OnRender() override {}
			void OnImgui() override  {}
			void OnEvent(Event& p_Event) override {}
	};



	Application* CreateApplication(int p_Argc, char** p_Argv)
	{
		KTN_INFO("Creating sandbox app...");

		auto app = new Application();


		app->PushLayer(CreateRef<SandboxLayer>());

		return app;
	}

} // namespace KTN