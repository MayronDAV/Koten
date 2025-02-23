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
		ImGui::Begin(m_Name.c_str(), &m_Active);

		if (m_Context)
		{
			m_Context->GetRegistry().view<TagComponent>().each(
			[&](auto p_Entt, const TagComponent& p_Tag)
			{
				Entity entt{ p_Entt , m_Context.get() };
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
					m_Context->CreateEntity("Empty Entity");

				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}

	void HierarchyPanel::DrawEnttNode(Entity p_Entt)
	{
		auto tag = p_Entt.GetName();

		auto hierarchyComponent = p_Entt.TryGetComponent<HierarchyComponent>();
		bool noChildren = true;
		if (hierarchyComponent != nullptr && hierarchyComponent->First != entt::null)
			noChildren = false;

		ImGuiTreeNodeFlags flags = (m_Editor->IsSelected(p_Entt) ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (noChildren)
			flags |= ImGuiTreeNodeFlags_Leaf;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)p_Entt, flags, tag.c_str());
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
