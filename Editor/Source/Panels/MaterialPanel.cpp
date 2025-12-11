#include "MaterialPanel.h"
#include "Editor.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>



namespace KTN
{
	MaterialPanel::MaterialPanel()
		: EditorPanel("Material")
	{
		m_Active = false;
	}

	void MaterialPanel::Open(AssetHandle p_Material)
	{
		m_Active = true;
		m_Changed = true;
		m_MaterialHandle = p_Material;
		m_Material = p_Material ? AssetManager::Get()->GetAsset(m_MaterialHandle) : nullptr;
	}

	void MaterialPanel::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		ImGui::Begin(m_Name.c_str(), &m_Active);

		ImGui::BeginGroup();

		if (m_Changed)
		{
			m_Material = m_MaterialHandle ? AssetManager::Get()->GetAsset(m_MaterialHandle) : nullptr;
			m_Metadata = AssetManager::Get()->GetMetadata(m_MaterialHandle);
			m_Changed = false;
		}

		ImGui::Text("Path");
		if (UI::InputText("#Path", m_Metadata.FilePath, false, 0, 2.0f, true))
			m_Edited = true;

		ImVec2 btnPos = ImGui::GetCursorScreenPos();
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 btnSize = ImVec2(100.0f, lineHeight);

		if (ImGui::Button(ICON_MDI_PLUS_BOX " New", btnSize))
		{
			m_MaterialHandle = AssetHandle();
			m_New = true;
			ImGui::OpenPopup("NewDropdownPopup");
		}

		ImGui::SetNextWindowPos(
			ImVec2(btnPos.x + btnSize.x + 4, btnPos.y), // 4px de margem
			ImGuiCond_Appearing
		);

		if (ImGui::BeginPopup("NewDropdownPopup"))
		{
			if (ImGui::Selectable("Copy"))
			{
				m_Metadata.FilePath = (Project::GetAssetFileSystemPath("Materials") / (FileSystem::GetStem(m_Metadata.FilePath) + "(1).ktasset")).string();
				m_Material->Handle = m_MaterialHandle;
			}

			if (ImGui::Selectable("PhysicsMaterial2D"))
			{
				m_Metadata.Type = AssetType::PhysicsMaterial2D;
				m_Metadata.FilePath = (Project::GetAssetFileSystemPath("Materials") / "New.ktasset").string();
				m_Material = CreateRef<PhysicsMaterial2D>();
				m_Material->Handle = m_MaterialHandle;
			}

			ImGui::EndPopup();
		}

		auto size = ImGui::GetContentRegionAvail();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::BeginChild("MaterialOptions", { size.x, size.y - (lineHeight * 1.2f) }, false);
		ImGui::PopStyleVar();
		if (m_Material)
		{
			auto type = m_Material->GetType();
			if (type == AssetType::PhysicsMaterial2D)
			{
				auto material = As<Asset, PhysicsMaterial2D>(m_Material);

				ImGui::InputFloat("Density", &material->Density);
				ImGui::InputFloat("Friction", &material->Friction);
				ImGui::InputFloat("Restitution", &material->Restitution);
				ImGui::InputFloat("RestitutionThreshold", &material->RestitutionThreshold);
			}
		}
		ImGui::EndChild();

		if (ImGui::Button("Save", { 100.0f, lineHeight }) && m_Material)
		{
			if (m_Edited)
			{
				if (AssetManager::Get()->IsAssetHandleValid(m_MaterialHandle))
				{
					auto& metadata = AssetManager::Get()->GetMetadata(m_MaterialHandle);
					metadata.FilePath = m_Metadata.FilePath;
					AssetManager::Get()->SerializeAssetRegistry();
				}
				m_Edited = false;
			}

			auto type = m_Material->GetType();
			if (type == AssetType::PhysicsMaterial2D)
			{
				if (m_New)
				{
					AssetManager::Get()->ImportAsset(m_MaterialHandle, m_Metadata, m_Material);
					m_New = false;
				}

				auto material = As<Asset, PhysicsMaterial2D>(m_Material);
				material->Serialize(m_Metadata.FilePath);
			}
		}

		ImGui::EndGroup();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				auto filepath = std::filesystem::path(path);
				if (filepath.extension() == ".ktasset")
				{
					m_MaterialHandle = AssetManager::Get()->ImportAsset(AssetType::Texture2D, filepath.string());
					m_Changed = true;
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::End();

	}


} // namespace KTN
