#include "SceneEditPanel.h"
#include "Editor.h"

// lib
#include <imgui_internal.h>


namespace KTN
{
	SceneEditPanel::SceneEditPanel()
		: EditorPanel("Scene Edit")
	{
		m_Active = false;
	}

	void SceneEditPanel::Create(AssetHandle p_Scene)
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_Active = true;
		m_SceneHandle = p_Scene;
		m_IsCreating = true;
		m_IsEditing = false;
		m_SceneFolder = (Project::GetAssetDirectory() / "Scenes").string();
		m_SceneName = "NewScene";
		m_SceneConfig = {};
	}

	void SceneEditPanel::Edit(AssetHandle p_Scene)
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_Active = true;
		m_SceneHandle = p_Scene;
		m_IsCreating = false;
		m_IsEditing = true;
		m_SceneFolder = AssetManager::Get()->GetMetadata(p_Scene).FilePath;
		m_SceneName = FileSystem::GetStem(m_SceneFolder);
		auto scene = AssetManager::Get()->GetAsset<Scene>(m_SceneHandle);
		m_SceneConfig = scene->GetConfig();
	}

	void SceneEditPanel::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		if (!m_Active || (!m_IsCreating && !m_IsEditing))
			return;

		ImGui::Begin(m_Name.c_str());

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		if (m_IsEditing)
			ImGui::BeginDisabled();

		UI::InputText("Folder", m_SceneFolder, true, 0, 2.0f, true);
		ImGui::SameLine();
		if (ImGui::Button(ICON_MDI_FOLDER_SEARCH, { 0.0f, lineHeight }))
		{
			std::string folderPath = "";
			if (FileDialog::PickFolder(m_SceneFolder, folderPath) == FileDialogResult::SUCCESS)
			{
				auto path = std::filesystem::path(folderPath);
				bool isDir = std::filesystem::is_directory(path);
				m_SceneFolder = isDir ? folderPath : path.parent_path().string();
			}
		}

		UI::InputText("Name ", m_SceneName, true, 0, 2.0f, true);

		bool canCreate = !m_SceneFolder.empty() && !m_SceneName.empty();
		if (!m_SceneName.empty())
		{
			std::string fullPath = m_SceneFolder + "/" + (m_SceneName + ".ktscn");
			if (std::filesystem::exists(fullPath))
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "A scene with this name already exists!");
				canCreate = false;
			}
		}

		if (m_IsEditing)
			ImGui::EndDisabled();

		auto size = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::BeginChild("##SceneConfig", { size.x, size.y - (lineHeight * 1.5f) }, false);
		ImGui::PopStyleVar();
		{
			ImGui::Checkbox("Use Physics 2D", &m_SceneConfig.UsePhysics2D);
		}
		ImGui::EndChild();

		if (!m_IsEditing && !canCreate)
			ImGui::BeginDisabled();

		if (ImGui::Button(m_IsEditing ? "Save" : "Create", {100.0f, lineHeight}))
		{
			std::string fullPath = m_SceneFolder + "/" + (m_SceneName + ".ktscn");

			if (m_IsCreating)
			{
				SceneManager::New(m_SceneHandle, fullPath, m_SceneConfig);
			}
			else if (m_IsEditing)
			{
				auto& metadata = AssetManager::Get()->GetMetadata(m_SceneHandle);
				metadata.FilePath = fullPath;

				auto scene = AssetManager::Get()->GetAsset<Scene>(m_SceneHandle);
				scene->GetConfig() = m_SceneConfig;
			}
			m_Active = false;
		}

		if (!m_IsEditing && !canCreate)
			ImGui::EndDisabled();

		ImGui::SameLine();

		if (ImGui::Button("Cancel", {100.0f, lineHeight}))
		{
			m_Active = false;
		}

		ImGui::End();
	}


} // namespace KTN
