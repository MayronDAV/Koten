#include "Editor.h"
#include "Koten/EntryPoint.h"



namespace KTN
{
	Application* CreateApplication(int p_Argc, char** p_Argv)
	{
		KTN_PROFILE_FUNCTION();

		auto app = new Application();
		ImGui::SetCurrentContext(app->GetImGui()->GetCurrentContext());

		app->PushLayer(CreateRef<Editor>());

		return app;
	}

} // namespace KTN