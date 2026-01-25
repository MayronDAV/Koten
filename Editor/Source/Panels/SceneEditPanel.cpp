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
	}

	void SceneEditPanel::Edit(AssetHandle p_Scene)
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_Active = true;
		m_SceneHandle = p_Scene;
		m_IsCreating = false;
		m_IsEditing = true;
	}

	void SceneEditPanel::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		if (!m_Active || (!m_IsCreating && !m_IsEditing))
			return;

		ImGui::Begin(m_Name.c_str());

		static std::string scenePath = (Project::GetAssetDirectory() / "Scenes" / "NewScene.ktscn").string();
		
		if (m_IsEditing)
		{
			ImGui::Text("Path");
			ImGui::SameLine();
			auto size = ImGui::GetContentRegionAvail();
			scenePath = AssetManager::Get()->GetMetadata(m_SceneHandle).FilePath;
			if (ImGui::Button(scenePath.c_str(), { size.x, 0.0f }))
			{
				std::string path = "";
				if (FileDialog::Open(".ktscn", Project::GetAssetDirectory().string(), path) == FileDialogResult::SUCCESS)
				{
					scenePath = path;
				}
			}
		}
		else
		{
			UI::InputText("Path", scenePath, true, 0, 2.0f, true);
		}

		SceneConfig config = {};
		if (m_IsEditing)
		{
			auto scene = AssetManager::Get()->GetAsset<Scene>(m_SceneHandle);
			config = scene->GetConfig();
		}

		auto size = ImGui::GetContentRegionAvail();
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::BeginChild("##SceneConfig", { size.x, size.y - (lineHeight * 1.5f) }, false);
		ImGui::PopStyleVar();
		{
			ImGui::Checkbox("Use Physics 2D", &config.UsePhysics2D);
		}
		ImGui::EndChild();

		if (ImGui::Button(m_IsEditing ? "Save" : "Create", {100.0f, lineHeight}))
		{
			if (m_IsCreating)
			{
				SceneManager::New(m_SceneHandle, scenePath, config);
			}
			else if (m_IsEditing)
			{
				auto& metadata = AssetManager::Get()->GetMetadata(m_SceneHandle);
				metadata.FilePath = scenePath;

				auto scene = AssetManager::Get()->GetAsset<Scene>(m_SceneHandle);
				scene->GetConfig() = config;
			}
			m_Active = false;
			m_IsCreating = false;
			m_IsEditing = false;
			m_SceneHandle = 0;
			scenePath = (Project::GetAssetDirectory() / "Scenes" / "NewScene.ktscn").string();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", {100.0f, lineHeight}))
		{
			m_Active = false;
			m_IsCreating = false;
			m_IsEditing = false;
			m_SceneHandle = 0;
			scenePath = (Project::GetAssetDirectory() / "Scenes" / "NewScene.ktscn").string();
		}

		ImGui::End();
	}


} // namespace KTN
