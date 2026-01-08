#include "HierarchyPanel.h"
#include "Editor.h"

// lib
#include <imgui.h>


namespace KTN
{
	HierarchyPanel::HierarchyPanel()
		: EditorPanel("Hierarchy")
	{
	}

	void HierarchyPanel::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		ImGui::Begin(m_Name.c_str(), &m_Active);

		const auto& scenes = SceneManager::GetActiveScenes();
		for (auto& scene : scenes)
		{
			std::string name = FileSystem::GetStem(AssetManager::Get()->GetMetadata(scene->Handle).FilePath);
			auto selectedScene = m_Editor->GetSelected().GetScene();
			ImGui::PushID((void*)(uint64_t)scene->Handle);

			ImGuiTreeNodeFlags flags = (selectedScene != nullptr && scene->Handle == selectedScene->Handle ? ImGuiTreeNodeFlags_Selected : 0)
				| ImGuiTreeNodeFlags_OpenOnArrow;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen;

			bool opened = ImGui::TreeNodeEx((void*)(uint64_t)scene->Handle, flags, name.c_str());

			if (opened)
			{
				scene->GetRegistry().view<TagComponent>().each(
				[&](auto p_Entt, const TagComponent& p_Tag)
				{
					Entity entt{ p_Entt , scene.get() };
					auto hierarchyComponent = entt.TryGetComponent<HierarchyComponent>();
					bool draw = hierarchyComponent ? hierarchyComponent->Parent == entt::null : true;
					if (draw)
						DrawEnttNode(entt);
				});

				if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
					m_Editor->UnSelectEntt();

				if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_NoOpenOverItems | 1))
				{
					if (ImGui::MenuItem("Create Empty Entity"))
						scene->CreateEntity("Empty Entity");

					ImGui::EndPopup();
				}

				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		ImGui::End();
	}

	void HierarchyPanel::DrawEnttNode(Entity p_Entt)
	{
		KTN_PROFILE_FUNCTION();

		auto tag = p_Entt.GetTag();

		auto hierarchyComponent = p_Entt.TryGetComponent<HierarchyComponent>();
		bool noChildren = true;
		if (hierarchyComponent != nullptr && hierarchyComponent->First != entt::null)
			noChildren = false;

		ImGuiTreeNodeFlags flags = (m_Editor->IsSelected(p_Entt) ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (noChildren)
			flags |= ImGuiTreeNodeFlags_Leaf;

		bool isPrefab = p_Entt.HasComponent<PrefabComponent>();
		if (isPrefab)
			ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 1.0f, 1.0f, 1.0f });

		{
			ImGui::PushID((uint32_t)p_Entt);
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.0f, 0.0f, 0.0f, 0.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.0f, 0.0f, 0.0f, 0.0f });

			auto& rc = p_Entt.GetComponent<RuntimeComponent>();
			if (ImGui::Button(rc.Active ? ICON_MDI_EYE : ICON_MDI_EYE_OFF))
				rc.Active = !rc.Active;

			ImGui::PopStyleColor(3);
			ImGui::PopID();
		}

		ImGui::SameLine();

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)p_Entt, flags, ((isPrefab ? ICON_MDI_CUBE " " : ICON_MDI_CUBE_OUTLINE " ") + tag).c_str());

		if (ImGui::BeginDragDropSource())
		{
			auto uuid = p_Entt.GetUUID();
			ImGui::SetDragDropPayload("HIERARCHY_ENTITY_ITEM", &uuid, sizeof(UUID));
			ImGui::EndDragDropSource();
		}

		if (isPrefab)
			ImGui::PopStyleColor();

		if (ImGui::IsItemClicked())
		{
			m_Editor->SetSelectedEntt(p_Entt);
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Add Empty Child"))
			{
				auto scene = p_Entt.GetScene();
				auto child = scene->CreateEntity("Empty Entity");
				child.SetParent(p_Entt);
			}

			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			auto& registry = p_Entt.GetScene()->GetRegistry();

			if (!noChildren)
			{
				entt::entity child = hierarchyComponent->First;
				while (child != entt::null && registry.valid(child))
				{
					auto childHerarchyComponent = registry.try_get<HierarchyComponent>(child);
					auto next = childHerarchyComponent ? childHerarchyComponent->Next : entt::null;
					DrawEnttNode(Entity(child, p_Entt.GetScene()));
					child = next;
				}
			}

			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			if (m_Editor->IsSelected(p_Entt))
				m_Editor->UnSelectEntt();

			p_Entt.Destroy();
		}
	}

} // namespace KTN
