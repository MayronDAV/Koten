#include "Editor.h"
#include "ProjectExplorer.h"
#include "Koten/EntryPoint.h"

// std
#include <iostream>



namespace KTN
{
	Application* CreateApplication(int p_Argc, char** p_Argv)
	{
		KTN_PROFILE_FUNCTION();

		Application* app = nullptr;

		for (int i = 0; i < p_Argc; i++)
		{
			if (strcmp(p_Argv[i], "--help") == 0 || strcmp(p_Argv[i], "--h") == 0)
			{
				std::cout << "Usage: \n";
				std::cout << "  --help[-h]                   | Show this message.\n";
				std::cout << "  --project[-p] [project_path] | Open the editor with the project path! If no project path is provided, the Project Explorer will be opened.\n";
				return app;
			}

			if (strcmp(p_Argv[i], "--project") == 0 || strcmp(p_Argv[i], "--p") == 0)
			{
				if (i + 1 < p_Argc)
				{
					std::string projectPath = std::filesystem::absolute(p_Argv[i + 1]).string();
					if (!projectPath.empty())
					{
						app = new Application();
						ImGui::SetCurrentContext(app->GetImGui()->GetCurrentContext());
						ImGuiIO& io = ImGui::GetIO(); (void)io;
						io.IniFilename = "Resources/Layout.ini";
						ImGui::LoadIniSettingsFromDisk(io.IniFilename);
						app->PushLayer(CreateRef<Editor>(projectPath));
						return app;
					}
				}
				else
				{
					std::cout << "Project path not provided after --project or --p flag.\n";
				}
			}
		}

		app = new Application();
		ImGui::SetCurrentContext(app->GetImGui()->GetCurrentContext());
		app->PushLayer(CreateRef<ProjectExplorer>());
		return app;
	}

} // namespace KTN