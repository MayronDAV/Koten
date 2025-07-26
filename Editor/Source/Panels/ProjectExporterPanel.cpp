#include "ProjectExporterPanel.h"
#include "Editor.h"

// lib
#include <imgui_internal.h>

// std
#ifdef KTN_WINDOWS
	#include <shlobj.h>
#elif defined(KTN_LINUX)
	#include <unistd.h>
	#include <sys/types.h>
	#include <pwd.h>
#endif



namespace KTN
{
	namespace
	{
		std::string GetSystemDocumentsFolder() 
		{
		#ifdef KTN_WINDOWS
			char path[MAX_PATH];
			if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path))) {
				return path;
			}
			return ""; // Fallback
		#elif defined(KTN_LINUX)
			const char* homeDir = getenv("HOME");
			if (homeDir == nullptr) {
				homeDir = getpwuid(getuid())->pw_dir;
			}

			// tries to find ~/Documents
			std::string documentsPath = std::string(homeDir) + "/Documents";
			if (access(documentsPath.c_str(), F_OK) != 0) {
				return homeDir;
			}
			return documentsPath;
		#endif

			return FileSystem::GetAbsolute("Documents");
		}

	} // namespace

	ProjectExporterPanel::ProjectExporterPanel()
		: EditorPanel("Project Exporter")
	{
		m_Active = false;
		m_FolderPath = GetSystemDocumentsFolder();
	}

	void ProjectExporterPanel::Open()
	{
		KTN_PROFILE_FUNCTION();

		m_Active = true;
		Clean();
	}

	void ProjectExporterPanel::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		ImGui::Begin(m_Name.c_str(), &m_Active);
		{

			ImGui::Text("Folder");
			ImGui::SameLine();
			if (ImGui::Button(m_FolderPath.c_str(), { ImGui::GetContentRegionAvail().x, 0.0f }))
			{
				std::string path = "";
				if (FileDialog::PickFolder("", path) == FileDialogResult::SUCCESS)
				{
					m_FolderPath = path;
				}
			}

			ImGui::NewLine();

			auto size = ImGui::GetContentRegionAvail();
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
			ImGui::BeginChild("##window_child", { size.x, size.y - (lineHeight * 1.5f) }, false);
			ImGui::PopStyleVar();
			{
				ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
				if (ImGui::BeginTabBar("ProjectExporterTabs", tab_bar_flags))
				{
					if (ImGui::BeginTabItem("Project"))
					{
						m_ChangedTab = m_Tab != 0;
						m_Tab = 0;

						ProjectConfig config = Project::GetActive()->GetConfig();
						if (m_First || m_ChangedTab)
							m_Config = config;

						UI::InputText("Name", m_Config.Name, true, 0, 2.0f, true);

						ImGui::Text("StartScene");
						ImGui::SameLine();
						std::string startScenePath = FileSystem::GetRelative(AssetManager::Get()->GetMetadata(m_Config.StartScene).FilePath, Project::GetAssetDirectory().string());
						if (ImGui::Button(startScenePath.c_str()))
						{
							std::string path = "";
							if (FileDialog::Open(".ktscn", "Assets", path) == FileDialogResult::SUCCESS)
							{
								m_Config.StartScene = AssetManager::Get()->ImportAsset(AssetType::Scene, path);
							}
						}

						// Icon Path
						{
							ImGui::NewLine();

							const char* const label = "Icon Path";
							ImVec2 imageSize = { 100.0f, 100.0f };
							ImVec2 buttonSize(imageSize.x / 2.0f, lineHeight);

							ImGui::Columns(2, "imageControlColumns", false);
							ImGui::SetColumnWidth(0, ImGui::CalcTextSize(label).x + 20.0f);

							ImGui::SetCursorPosY(ImGui::GetCursorPosY() + imageSize.y * 0.5f - ImGui::GetTextLineHeight() * 0.5f);
							ImGui::Text(label);

							ImGui::NextColumn();

							ImGui::BeginGroup();
							{
								ImGui::SetNextItemAllowOverlap();
								if (m_Icon)
								{
									UI::Image(m_Icon, imageSize);
								}
								else
								{
									ImGui::InvisibleButton("##img_placeholder", imageSize);
									ImDrawList* draw_list = ImGui::GetWindowDrawList();
									ImVec2 p0 = ImGui::GetItemRectMin();
									ImVec2 p1 = ImGui::GetItemRectMax();

									draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_FrameBg));

									const char* const placeholderText = "None";
									ImVec2 text_size = ImGui::CalcTextSize(placeholderText);
									ImVec2 text_pos(
										p0.x + (imageSize.x - text_size.x) * 0.5f,
										p0.y + (imageSize.y - text_size.y) * 0.5f
									);
									draw_list->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), placeholderText);
								}

								ImGui::SetCursorPosX(ImGui::GetCursorPosX() + imageSize.x - buttonSize.x);
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (buttonSize.y * 1.25f));

								if (ImGui::Button("Change", buttonSize))
								{
									std::string path = "";
									if (FileDialog::Open("", Project::GetAssetDirectory().string(), path) == FileDialogResult::SUCCESS)
									{
										m_Config.IconPath = path;
										m_Icon = TextureImporter::LoadTexture2D(m_Config.IconPath);
									}
								}
							}
							ImGui::EndGroup();

							ImGui::Columns(1);
						}
						
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Settings"))
					{
						m_ChangedTab = m_Tab != 1;
						m_Tab = 1;

						ImGui::Checkbox("Auto recompile scripts", &m_Settings.AutoRecompile);
						ImGui::Checkbox("Update when minimized", &m_Settings.UpdateMinimized);

						ImGui::NewLine();

						int currentItem = (int)m_Settings.Mode;
						static const char* items[] = { "Windowed", "Fullscreen", "Borderless" };
						static const int itemsCount = IM_ARRAYSIZE(items);
						if (UI::Combo("Mode", items[currentItem], items, itemsCount, &currentItem, -1))
						{
							m_Settings.Mode = (WindowMode)currentItem;
						}

						ImGui::InputScalar("Width", ImGuiDataType_U32, &m_Settings.Width);
						ImGui::InputScalar("Height", ImGuiDataType_U32, &m_Settings.Height);

						ImGui::NewLine();

						ImGui::Checkbox("Resizable", &m_Settings.Resizable);
						ImGui::Checkbox("Maximize", &m_Settings.Maximize);
						ImGui::Checkbox("Center", &m_Settings.Center);
						ImGui::Checkbox("Vsync", &m_Settings.Vsync);

						ImGui::EndTabItem();
					}
				
					ImGui::EndTabBar();
				}
			}
			ImGui::EndChild();

			if (ImGui::Button("Export", { 100.0f, lineHeight }))
			{
				ExportProject(m_FolderPath);
				Clean();
				m_Active = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", { 100.0f, lineHeight }))
			{
				Clean();
				m_Active = false;
			}
		}
		ImGui::End();

		m_First = false;
	}

	void ProjectExporterPanel::ExportProject(const std::string& p_Folder)
	{
		KTN_PROFILE_FUNCTION();

		auto folder = (std::filesystem::path)FileSystem::GetAbsolute(p_Folder) / m_Config.Name;
		if (FileSystem::Exists((folder).string()))
			FileSystem::Remove(folder.string());

	#ifdef KTN_WINDOWS
		FileSystem::Copy(FileSystem::GetAbsolute("Resources/Runtime/Runtime.exe"), folder.string());
		FileSystem::Copy(FileSystem::GetAbsolute("Resources/Runtime/Koten.dll"), folder.string());
		FileSystem::Rename((folder / "Runtime.exe").string(), m_Config.Name + ".exe");
	#elif defined(KTN_LINUX)
		FileSystem::Copy(FileSystem::GetAbsolute("Resources/Runtime/Runtime"), folder.string());
		FileSystem::Copy(FileSystem::GetAbsolute("Resources/Runtime/Koten.so"), folder.string());
		FileSystem::Rename((folder / "Runtime").string(), m_Config.Name);
	#endif

		if (!m_Config.IconPath.empty())
			FileSystem::Copy(m_Config.IconPath, (folder).string());

		bool existsScriptFiles = false;
		for (const auto& entry : std::filesystem::directory_iterator(Project::GetAssetFileSystemPath("Scripts")))
		{
			if (entry.path().extension() == ".cs")
			{
				existsScriptFiles = true;
				break;
			}
		}

		auto scriptsDllpath = Project::GetAssetFileSystemPath("Scripts.dll").string();
		if (existsScriptFiles || FileSystem::Exists(scriptsDllpath))
		{
			if (m_Settings.AutoRecompile)
				FileSystem::Copy(FileSystem::GetAbsolute("Mono"), (folder / "Mono").string());
			else
				FileSystem::Copy(FileSystem::GetAbsolute("Mono/lib"), (folder / "Mono/lib").string());

			FileSystem::Copy(FileSystem::GetAbsolute("Resources/Scripts/Koten-ScriptCore.dll"), (folder / "Resources/Scripts").string());
		}

		if (m_Settings.AutoRecompile)
			FileSystem::Copy(Project::GetAssetFileSystemPath("Scripts").string(), (folder / m_Config.AssetDirectory / "Scripts").string());
		else if (FileSystem::Exists(scriptsDllpath))
			FileSystem::Copy(scriptsDllpath, (folder / m_Config.AssetDirectory).string());
		else if (existsScriptFiles)
			ScriptEngine::CompileScripts(Project::GetAssetFileSystemPath("Scripts"), folder / m_Config.AssetDirectory);

		FileSystem::Copy(FileSystem::GetAbsolute("Assets/Shaders"), (folder / m_Config.AssetDirectory / "Shaders").string());
		FileSystem::Copy(Project::GetAssetFileSystemPath("Fonts").string(), (folder / m_Config.AssetDirectory / "Fonts").string());
		FileSystem::Copy(FileSystem::GetAbsolute("Assets/Fonts"), (folder / m_Config.AssetDirectory / "Fonts").string());
		FileSystem::Copy(Project::GetAssetFileSystemPath("Textures").string(), (folder / m_Config.AssetDirectory / "Textures").string());

		auto proj = CreateRef<Project>(m_Config);
		ProjectSerializer serializer(proj);
		serializer.SerializeRuntime(folder / "Data.ktdt");

		Engine eng{};
		eng.GetSettings() = m_Settings;
		eng.SaveSettings(folder / "Resources");

		AssetManager::Get()->SerializeAssetPack(folder / m_Config.AssetDirectory);
	}

	void ProjectExporterPanel::Clean()
	{
		m_Settings = {};
		m_Settings.AutoRecompile = false;
		m_Config = {};
		m_First = true;
		m_ChangedTab = false;
	}
}
