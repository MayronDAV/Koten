#include "SettingsPanel.h"
#include "Editor.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>



namespace KTN
{
	SettingsPanel::SettingsPanel()
		: EditorPanel("Settings")
	{
		m_Active = false;
	}

	void SettingsPanel::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
		ImGui::Begin(m_Name.c_str(), &m_Active);
		{
			ImGuiStyle& style = ImGui::GetStyle();
			auto padding = style.WindowPadding;

			ImVec2 availableSize = ImGui::GetContentRegionAvail();

			float widget1Width = availableSize.x * 0.25f;

			float widget2Width = availableSize.x - widget1Width - padding.x;

			ImGui::BeginChild("##Tabs", ImVec2(widget1Width, availableSize.y), true, ImGuiWindowFlags_None);
			{
				for (const auto& [tabHeader, tabs] : m_Tabs)
				{
					if (DrawNode(tabHeader, true))
					{
						for (const auto& tab : tabs)
						{
							DrawNode(tab.Name, false);
							ImGui::TreePop();
						}

						ImGui::TreePop();
					}
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("##TabContent", ImVec2(widget2Width, availableSize.y), true, ImGuiWindowFlags_None);
			{
				for (const auto& [tabHeader, tabs] : m_Tabs)
				{
					for (const auto& tab : tabs)
					{
						if (m_Selected == tab.Name)
						{
							tab.Content();
						}
					}
				}
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}

	bool SettingsPanel::DrawNode(const std::string& p_Name, bool p_IsHeader)
	{
		ImGuiTreeNodeFlags flags = (m_Selected == p_Name ? ImGuiTreeNodeFlags_Selected : 0)
			| ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (!p_IsHeader)
			flags |= ImGuiTreeNodeFlags_Leaf;

		bool opened = ImGui::TreeNodeEx(p_Name.c_str(), flags, p_Name.c_str());
		if (ImGui::IsItemClicked())
		{
			m_Selected = p_Name;
		}

		return opened;
	}

} // namespace KTN
