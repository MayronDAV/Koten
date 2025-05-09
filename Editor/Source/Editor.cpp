#include "Editor.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/SceneViewPanel.h"
#include "Panels/SettingsPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Shortcuts.h"

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
		static void BeginDockspace(bool p_MenuBar)
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
			ImGui::Begin("EditorDockspace", nullptr, window_flags);
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
		
		static void EndDockspace()
		{
			KTN_PROFILE_FUNCTION();

			ImGui::End();
		}

	} // namespace

	Editor::Editor()
		: Layer("Editor")
	{
		auto file = IniFile("Resources/Engine.ini");
		
		{
			auto& settings = Engine::GetSettings();

			{
				file.Add<bool>("Settings", "Mouse Picking", settings.MousePicking);
				file.Rewrite();
			}

			settings.MousePicking = file.Get<bool>("Settings", "Mouse Picking");
		}

		Shortcuts::Init(file);
	}

	Editor::~Editor()
	{
	}

	void Editor::OpenScene(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		auto scene = CreateRef<Scene>();

		SceneSerializer serializer(scene);
		if (!serializer.Deserialize(p_Path))
			KTN_CORE_ERROR("Failed to Deserialize!");

		UnSelectEntt();
		m_ActiveScene = scene;

		for (auto& panel : m_Panels)
		{
			panel->SetContext(m_ActiveScene);
		}
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

		auto contentBrowser = CreateRef<ContentBrowserPanel>();
		m_Panels.emplace_back(contentBrowser);


		for (auto& panel : m_Panels)
		{
			panel->SetContext(m_ActiveScene);
			panel->SetEditor(this);
		}

		#pragma region Settings

		Tab tab;

		// General
		{
			static const char* group_name = "General";

			// Editor Camera
			{
				tab.Name = "Editor Camera";
				tab.Content = [&]()
				{
					int currentItem = (int)m_Camera->GetMode();
					static const char* items[] = { "Two Dim", "Fly Cam" };
					static const int itemsCount = IM_ARRAYSIZE(items);

					if (UI::Combo("Mode", items[currentItem], items, itemsCount, &currentItem, 50.0f))
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

				m_Settings->AddTab(group_name, tab);
			}

			// Shortcuts
			{
				tab.Name = "Shortcuts";
				tab.Content = [&]()
				{
					std::unordered_map<std::string, std::vector<int>>& shortcuts = Shortcuts::GetShortcuts();

					static std::unordered_map<std::string, std::string> lastShortcuts;
					static bool showCaptureWindow = false;
					static std::string editAction = "";

					auto size = ImGui::GetContentRegionAvail();

					float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
					ImGui::BeginChild("##shortcuts_child", { size.x, size.y - (lineHeight * 1.5f) }, false);
					ImGui::PopStyleVar();

					ImGui::Columns(2, "##shortcuts_table", false);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::SetColumnWidth(1, 250.0f);

					if (lastShortcuts.empty())
					{
						for (auto& [action, keys] : shortcuts)
							lastShortcuts[action] = Shortcuts::KeysToString(keys);
					}

					for (auto& [action, keys] : shortcuts)
					{
						ImGui::Text(action.c_str());
						ImGui::NextColumn();

						std::string keyCombination = Shortcuts::KeysToString(keys);

						if (ImGui::Button(("##button_" + action).c_str(), ImVec2(200.0f, lineHeight)))
						{
							editAction = action;
							showCaptureWindow = true;
							m_CaptureShortcuts = false;
						}

						auto cursor = ImGui::GetCursorPos();

						ImVec2 buttonPos = ImGui::GetItemRectMin();
						ImVec2 rectSize = ImGui::GetItemRectSize();

						// Reset button
						{
							ImGui::SameLine();

							if (ImGui::Button((ICON_MDI_RESTART "##reset_" + action).c_str(), { lineHeight, lineHeight }))
							{
								if (keyCombination != lastShortcuts[action])
								{
									keyCombination = lastShortcuts[action];
									Shortcuts::SetShortcut(action, Shortcuts::StringToKeys(keyCombination));
								}
							}
						}

						ImVec2 textSize = ImGui::CalcTextSize(keyCombination.c_str());
						ImGui::SetCursorScreenPos(ImVec2(
							buttonPos.x + (rectSize.x - textSize.x) * 0.5f,
							buttonPos.y + (rectSize.y - textSize.y) * 0.5f
						));
						ImGui::TextUnformatted(keyCombination.c_str());

						ImGui::NextColumn();
					}

					ImGui::Columns(1);

					ImGui::EndChild();

					if (ImGui::Button("Save", { 100.0f, lineHeight }))
					{
						auto file = IniFile("Resources/Engine.ini"); // TODO
						Shortcuts::UploadShortcuts(file);
					}

					if (showCaptureWindow && !editAction.empty())
					{
						static std::vector<int> keys;
						static std::string keyCombination = "";

						ImGui::OpenPopup("Capture Shortcut");
						if (ImGui::BeginPopupModal("Capture Shortcut", NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							ImGui::Text("Press the key combination you want to capture...");

							ImGui::Separator();

							int key = Input::GetKeyPressed();
							if (key != -1)
							{
								if (std::find(keys.begin(), keys.end(), key) == keys.end())
									keys.push_back(key);
							}

							if (!keys.empty())
								keyCombination = Shortcuts::KeysToString(keys);

							ImGui::Text("Keys: %s", keyCombination.c_str());

							if (ImGui::Button("Ok"))
							{
								lastShortcuts[editAction] = Shortcuts::GetShortcutStr(editAction);
								Shortcuts::SetShortcut(editAction, keys);
								editAction = "";
								keyCombination = "";
								showCaptureWindow = false;
								m_CaptureShortcuts = true;
								keys.clear();
							}

							ImGui::SameLine();

							if (ImGui::Button("Cancel"))
							{
								editAction = "";
								keyCombination = "";
								showCaptureWindow = false;
								m_CaptureShortcuts = true;
								keys.clear();
							}
							ImGui::EndPopup();
						}
					}
				};

				m_Settings->AddTab(group_name, tab);
			}
		}

		// Panels
		{
			static const char* group_name = "Panels";

			m_Settings->AddTab(group_name, contentBrowser->GetSettingsTab());
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

		Shortcuts();

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
				auto shortcut = Shortcuts::GetShortcutStr("Save Scene As");
				if (ImGui::MenuItem("Save Scene As", shortcut.c_str()))
					SaveSceneAs();

				shortcut = Shortcuts::GetShortcutStr("Open Scene");
				if (ImGui::MenuItem("Open Scene", shortcut.c_str()))
					OpenScene();

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
				auto shortcut = Shortcuts::GetShortcutStr("Open Settings");
				if (ImGui::MenuItem(ICON_MDI_COGS "  Settings...", shortcut.c_str()))
					m_Settings->SetActive(!m_Settings->IsActive());
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void Editor::Shortcuts()
	{
		if (!m_CaptureShortcuts)
			return;

		if (Shortcuts::IsActionPressed("Open Scene"))
			OpenScene();

		if (Shortcuts::IsActionPressed("Save Scene As"))
			SaveSceneAs();

		if (Shortcuts::IsActionPressed("Open Settings"))
			m_Settings->SetActive(true); // Maybe change this to toggle in the future
	}

	void Editor::OpenScene()
	{
		std::string path = "";
		if (FileDialog::Open("", "Assets", path) == FileDialogResult::SUCCESS)
		{
			OpenScene(path);
		}
	}

	void Editor::SaveSceneAs()
	{
		std::string path = "";
		if (FileDialog::Save("", "Assets", path) == FileDialogResult::SUCCESS)
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(path);
		}
	}

} // namespace KTN

