#include "Editor.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/SceneViewPanel.h"
#include "Panels/SettingsPanel.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo.h>




namespace KTN
{
	namespace
	{
		void BeginDockspace(bool p_MenuBar)
		{
			KTN_PROFILE_FUNCTION();

			static bool opt_fullscreen = true;
			static bool opt_padding = false;
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
			if (p_MenuBar)
				window_flags |= ImGuiWindowFlags_MenuBar;

			if (opt_fullscreen)
			{
				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->WorkPos);
				ImGui::SetNextWindowSize(viewport->WorkSize);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			}
			else
			{
				dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
			}

			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 1.0f });
			if (!opt_padding)
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("SandboxDockspace", nullptr, window_flags);
			if (!opt_padding)
				ImGui::PopStyleVar();
			ImGui::PopStyleColor(); // windowBg

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// Submit the DockSpace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}
		}

		void EndDockspace()
		{
			KTN_PROFILE_FUNCTION();

			ImGui::End();
		}

	} // namespace

	Editor::Editor()
		: Layer("Editor")
	{
	}

	Editor::~Editor()
	{
	}

	void Editor::OnAttach()
	{
		KTN_PROFILE_FUNCTION();

		m_Camera = CreateRef<EditorCamera>();

		ImGuizmo::Init();

		m_ActiveScene = CreateRef<Scene>();

		m_Settings = CreateRef<SettingsPanel>();

		m_Panels.emplace_back(CreateRef<SceneViewPanel>());
		m_Panels.emplace_back(CreateRef<HierarchyPanel>());
		m_Panels.emplace_back(CreateRef<InspectorPanel>());


		for (auto& panel : m_Panels)
		{
			panel->SetContext(m_ActiveScene);
			panel->SetEditor(this);
		}

		#pragma region Settings

		static const char* general = "General";

		Tab tab;
		{
			tab.Name = "Editor Camera";
			tab.Content = [&]()
			{
				int currentItem = (int)m_Camera->GetMode();
				static const char* items[] = { "Two Dim", "Fly Cam" };
				static const int itemsCount = IM_ARRAYSIZE(items);

				if (UI::Combo("Mode",  items[currentItem], items, itemsCount, &currentItem, 50.0f))
				{
					m_Camera->SetMode((EditorCameraMode)currentItem);
				}

				float speed = m_Camera->GetSpeed();
				if (ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 0.0f, "%.2f"))
					m_Camera->SetSpeed(speed);

				float sensitivity = m_Camera->GetSensitivity();
				if (ImGui::DragFloat("Sensitivity", &sensitivity, 0.1f, 0.0f, 0.0f, "%.2f"))
					m_Camera->SetSensitivity(sensitivity);
			};

			m_Settings->AddTab(general, tab);
		}


		#pragma endregion
	}

	void Editor::OnDetach()
	{
		UnSelectEntt();

		ImGuizmo::Shutdown();
	}

	void Editor::OnUpdate()
	{
		KTN_PROFILE_FUNCTION();

		m_Camera->OnUpdate();

		for (auto& panel : m_Panels)
		{
			if (panel->IsActive())
				panel->OnUpdate();
		}
	}

	void Editor::OnRender()
	{
		KTN_PROFILE_FUNCTION();

		for (auto& panel : m_Panels)
		{
			if (panel->IsActive())
				panel->OnRender();
		}
	}

	void Editor::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		BeginDockspace(true);

		ImGuizmo::BeginFrame();

		DrawMenuBar();

		if (m_Settings->IsActive())
			m_Settings->OnImgui();

		for (auto& panel : m_Panels)
		{
			if (panel->IsActive())
				panel->OnImgui();
		}

		EndDockspace();
	}

	void Editor::OnEvent(Event& p_Event)
	{

	}

	void Editor::DrawMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save Scene As"))
				{
					std::string path = "";
					if (FileDialog::Save("", "Assets", path) == FileDialogResult::SUCCESS)
					{
						SceneSerializer serializer(m_ActiveScene);
						serializer.Serialize(path);
					}
				}

				if (ImGui::MenuItem("Load Scene"))
				{
					auto scene = CreateRef<Scene>();
					std::string path = "";
					if (FileDialog::Open("", "Assets", path) == FileDialogResult::SUCCESS)
					{
						SceneSerializer serializer(scene);
						if (!serializer.Deserialize(path))
							KTN_CORE_ERROR("Failed to Deserialize!");

						UnSelectEntt();
						m_ActiveScene = scene;

						for (auto& panel : m_Panels)
						{
							panel->SetContext(m_ActiveScene);
						}
					}
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
					Application::Get().Close();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Panels"))
			{
				for (auto& panel : m_Panels)
				{
					std::string name = panel->GetName();
					bool& active = panel->IsActive();
					ImGui::MenuItem(name.c_str(), nullptr, &active);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem(ICON_MDI_COGS "  Settings"))
					m_Settings->SetActive(!m_Settings->IsActive());
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

} // namespace KTN

