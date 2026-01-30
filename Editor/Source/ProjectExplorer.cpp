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

        if (m_State == ProjectExplorerState::MainMenu)
        {
            ShowMainMenu({ size.x, size.y });
        }
        else if (m_State == ProjectExplorerState::CreatingProject)
        {
            ShowCreateProjectForm({ size.x, size.y });
        }

        ImGui::End();
        Editor::EndDockspace();
    }

    void ProjectExplorer::ShowMainMenu(const glm::vec2& p_Size)
    {
        KTN_PROFILE_FUNCTION();

        auto title = "Project Explorer";
        auto subTitle = "( Work in Progress! )";
        ImGui::SetCursorPosX((p_Size.x - CalcTextSizeA(title).x) / 2.0f);
        ImGui::Text(title);
        ImGui::SetCursorPosX((p_Size.x - CalcTextSizeA(subTitle).x) / 2.0f);
        ImGui::Text(subTitle);

        ImGui::BeginChild("ProjectList", ImVec2(p_Size.x / 2.0f, 0.0f), false);
        {
            ImGui::Text("Projects list under construction, open or create a project to continue.");
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("ProjectOptions", ImVec2(p_Size.x / 2.0f, 0.0f), false);
        {
            if (ImGui::Button("New Project", ImVec2(p_Size.x / 2.1f, 50.0f)))
            {
                m_NewProjectFolder = FileSystem::GetAbsolute("Projects");
                if (FileSystem::Exists(m_NewProjectFolder) == false)
                {
                    FileSystem::CreateDirectories(m_NewProjectFolder);
				}

                m_NewProjectName = "";
                m_State = ProjectExplorerState::CreatingProject;
            }

            if (ImGui::Button("Open Project", ImVec2(p_Size.x / 2.1f, 50.0f)))
            {
                std::string path = "";
                if (FileDialog::Open(".ktproj", "", path) == FileDialogResult::SUCCESS)
                {
                    m_ShouldOpenProject = true;
                    m_ProjectPath = path;
                }
            }
        }
        ImGui::EndChild();
    }

    void ProjectExplorer::ShowCreateProjectForm(const glm::vec2& p_Size)
    {
        KTN_PROFILE_FUNCTION();

        ImGui::SetCursorPosX((p_Size.x - CalcTextSizeA("Create New Project").x) / 2.0f);
        ImGui::Text("Create New Project");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

        UI::InputText("Folder", m_NewProjectFolder, true, 0, 2.0f, true);
        auto line = CalcTextSizeA("Folder");
        ImGui::SameLine();
        if (ImGui::Button(ICON_MDI_FOLDER_SEARCH, { 0.0f, lineHeight }))
        {
            std::string folderPath = "";
            if (FileDialog::PickFolder(m_NewProjectFolder, folderPath) == FileDialogResult::SUCCESS)
            {
                auto path = std::filesystem::path(folderPath);
				bool isDir = std::filesystem::is_directory(path);
                m_NewProjectFolder = isDir ? folderPath : path.parent_path().string();
            }
        }

        UI::InputText("Name ", m_NewProjectName, true, 0, 2.0f, true);

        if (!m_NewProjectName.empty())
        {
            std::string fullPath = m_NewProjectFolder + "/" + m_NewProjectName;
            if (std::filesystem::exists(fullPath))
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "A project with this name already exists!");
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SetCursorPosX(p_Size.x / 2 - 125);

        if (ImGui::Button("Cancel", ImVec2(120, 40)))
        {
            m_State = ProjectExplorerState::MainMenu;
            m_NewProjectFolder = "";
            m_NewProjectName = "";
        }

        ImGui::SameLine();

        bool canCreate = !m_NewProjectName.empty() && !m_NewProjectFolder.empty();
        if (!canCreate)
            ImGui::BeginDisabled();

        if (ImGui::Button("Create Project", ImVec2(120, 40)))
        {
            CreateProject();
        }

        if (!canCreate)
            ImGui::EndDisabled();
    }

    void ProjectExplorer::CreateProject()
    {
        KTN_PROFILE_FUNCTION();

        if (m_NewProjectName.empty() || m_NewProjectFolder.empty())
            return;

        std::string projectPath = m_NewProjectFolder + "/" + m_NewProjectName;
        if (!std::filesystem::exists(projectPath))
            std::filesystem::create_directory(projectPath);

        auto project = Project::New(projectPath);
        if (project)
        {
            m_ShouldOpenProject = true;
            m_ProjectPath = (project->GetProjectDirectory() / (m_NewProjectName + ".ktproj")).string();
        }

        m_NewProjectFolder = "";
        m_NewProjectName = "";
        m_State = ProjectExplorerState::MainMenu;
    }
    
} // namespace KTN