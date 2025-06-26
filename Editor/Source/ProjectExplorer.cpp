#include "ProjectExplorer.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>



namespace KTN
{
	namespace
	{
		ImVec2 CalcTextSizeA(const char* p_Text)
		{
			return ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, 0.0f, p_Text);
		}

	} // namespace

	ProjectExplorer::ProjectExplorer()
		: Layer("ProjectExplorer")
	{
	}

	ProjectExplorer::~ProjectExplorer()
	{
	}

	void ProjectExplorer::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		if (m_ShouldOpenProject)
		{
			static bool first = true;
			if (!first)
				return;

			Application::Get().SubmitToMainThread([&]()
			{
				m_ShouldOpenProject = false;
				ImGuiIO& io = ImGui::GetIO(); (void)io;
				io.IniFilename = "Resources/Layout.ini";
				ImGui::LoadIniSettingsFromDisk(io.IniFilename);
				Application::Get().PushLayer(CreateRef<Editor>(m_ProjectPath));
				Application::Get().GetLayerStack().PopLayer(0);
			});

			first = false;
			return;
		}

		Editor::BeginDockspace("ProjectExplorerDockspaceID", "ProjectExplorerDockspace", false, ImGuiDockNodeFlags_NoTabBar);
		auto dockID = ImGui::GetID("ProjectExplorerDockspaceID");

		ImGui::SetNextWindowDockID(dockID, ImGuiCond_Always);
		ImGui::Begin("ProjectExplorer");

		auto size = ImGui::GetContentRegionAvail();

		auto title = "Project Explorer";
		auto subTitle = "( Work in Progress! )";
		ImGui::SetCursorPosX((size.x - CalcTextSizeA(title).x) / 2.0f);
		ImGui::Text(title);
		ImGui::SetCursorPosX((size.x - CalcTextSizeA(subTitle).x) / 2.0f);
		ImGui::Text(subTitle);

		ImGui::BeginChild("ProjectList", ImVec2(size.x / 2.0f, 0.0f), false);
		{
			ImGui::Text("Projects list under construction, open or create a project to continue.");
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("ProjectOptions", ImVec2(size.x / 2.0f, 0.0f), false);
		{
			static std::string folderPath = "";

			if (ImGui::Button("New Project", ImVec2(size.x / 2.1f, 50.0f)))
			{
				if (FileDialog::PickFolder("", folderPath) == FileDialogResult::SUCCESS)
				{
					ImGui::OpenPopup("New Project");
				}
			}

			if (ImGui::Button("Open Project", ImVec2(size.x / 2.1f, 50.0f)))
			{
				std::string path = "";
				if (FileDialog::Open(".ktproj", "", path) == FileDialogResult::SUCCESS)
				{
					m_ShouldOpenProject = true;
					m_ProjectPath = path;
				}
			}

			if (ImGui::BeginPopup("New Project") && !folderPath.empty())
			{
				static std::string projectName = "";
				UI::InputText("Project Name", projectName, true);
				
				if (ImGui::Button("Create", ImVec2(120, 0)))
				{
					if (!std::filesystem::exists(folderPath + "/" + projectName))
						std::filesystem::create_directory(folderPath + "/" + projectName);

					auto project = Project::New(folderPath + "/" + projectName);
					if (project)
					{
						m_ShouldOpenProject = true;
						m_ProjectPath = (project->GetProjectDirectory() / "Project.ktproj").string();
					}
					folderPath = "";
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Cancel", ImVec2(120, 0)))
				{
					folderPath = "";
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		ImGui::EndChild();

		ImGui::End();

		Editor::EndDockspace();
	}

} // namespace KTN