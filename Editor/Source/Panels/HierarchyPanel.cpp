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
		ImGui::Begin(m_Name.c_str());

		if (m_Context)
		{
			m_Context->GetRegistry().view<TagComponent>().each(
			[&](auto p_Entt, const TagComponent& p_Tag)
			{
				Entity entt{ p_Entt , m_Context.get() };
				DrawEnttNode(entt);
			});

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_Editor->UnSelectEntt();

			if (ImGui::BeginPopupContextWindow(0, 1))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					m_Context->CreateEntity("Empty Entity");

				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}

	void HierarchyPanel::DrawEnttNode(Entity p_Entt)
	{
		auto& tag = p_Entt.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = (m_Editor->IsSelected(p_Entt) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)p_Entt, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_Editor->SetSelectedEntt(p_Entt);
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, "Fake child");
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}
	}

} // namespace KTN
