#include "ktnpch.h"
#include "UI.h"

#include "Platform/OpenGL/GLTexture.h"

// lib
#include <imgui_internal.h>



namespace KTN::UI
{
	KTN_API ImTextureID AddImage(const Ref<Texture2D>& p_Texture)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return (ImTextureID)(uint64_t)As<Texture2D, GLTexture2D>(p_Texture)->GetID();

		KTN_CORE_ERROR("Unsupported API!");
		return ImTextureID(0);
	}

	KTN_API void Image(const Ref<Texture2D>& p_Texture, const ImVec2& p_Size, const ImVec4& p_TintColor, const ImVec4& p_BorderColor)
	{
		KTN_PROFILE_FUNCTION();

		Image(p_Texture, p_Size, { 0, 1 }, { 1, 0 }, p_TintColor, p_BorderColor);
	}
	KTN_API void Image(const Ref<Texture2D>& p_Texture, const ImVec2& p_Size, const ImVec2& p_UV0, const ImVec2& p_UV1, const ImVec4& p_TintColor, const ImVec4& p_BorderColor)
	{
		KTN_PROFILE_FUNCTION();

		ImGui::Image(AddImage(p_Texture), p_Size, p_UV0, p_UV1, p_TintColor, p_BorderColor);
	}

	KTN_API void ImageButton(const Ref<Texture2D>& p_Texture, const ImVec2& p_Size)
	{
		KTN_PROFILE_FUNCTION();

		std::string id = "##" + p_Texture->GetSpecification().DebugName;
		ImGui::ImageButton(id.c_str(), AddImage(p_Texture), p_Size, {0, 1}, {1, 0});
	}

	void ImageButton(const Ref<Texture2D>& p_Texture, const ImVec2& p_Size, const ImVec2& p_UV0, const ImVec2& p_UV1)
	{
		KTN_PROFILE_FUNCTION();

		std::string id = "##" + p_Texture->GetSpecification().DebugName;
		ImGui::ImageButton(id.c_str(), AddImage(p_Texture), p_Size, p_UV0, p_UV1);
	}

	KTN_API bool InputText(std::string& p_Text, const char* p_ID)
	{
		KTN_PROFILE_FUNCTION();

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
		char buffer[256];
		memset(buffer, 0, 256);
		memcpy(buffer, p_Text.c_str(), p_Text.length());
		ImGui::PushID(p_ID);
		bool updated = ImGui::InputText("##SceneName", buffer, 256);

		DrawItemActivityOutline(2.0f, false);

		if (updated)
			p_Text = std::string(buffer);

		ImGui::PopID();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		return updated;
	}

	KTN_API void DrawItemActivityOutline(float p_Rounding, bool p_DrawWhenInactive, ImColor p_ColorWhenActive)
	{
		KTN_PROFILE_FUNCTION();

		auto* drawList = ImGui::GetWindowDrawList();

		ImRect expandedRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		expandedRect.Min.x -= 1.0f;
		expandedRect.Min.y -= 1.0f;
		expandedRect.Max.x += 1.0f;
		expandedRect.Max.y += 1.0f;

		const ImRect rect = expandedRect;
		if (ImGui::IsItemHovered() && !ImGui::IsItemActive())
		{
			drawList->AddRect(rect.Min, rect.Max,
				ImColor(60, 60, 60), p_Rounding, 0, 1.5f);
		}
		if (ImGui::IsItemActive())
		{
			drawList->AddRect(rect.Min, rect.Max,
				p_ColorWhenActive, p_Rounding, 0, 1.0f);
		}
		else if (!ImGui::IsItemHovered() && p_DrawWhenInactive)
		{
			drawList->AddRect(rect.Min, rect.Max,
				ImColor(50, 50, 50), p_Rounding, 0, 1.0f);
		}
	}

	KTN_API bool ColorEdit4(const std::string& p_Label, glm::vec4& p_Values, float p_ResetValue)
	{
		KTN_PROFILE_FUNCTION();

		ImGui::PushID(p_Label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 100.0f); // TODO: Maybe change this?
		ImGui::Text(p_Label.c_str());
		ImGui::NextColumn();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::Text("");

		ImGui::SameLine();
		bool result = ImGui::ColorEdit4("##RGBA", glm::value_ptr(p_Values));


		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();

		return result;
	}

	KTN_API void DragFloat3(const std::string& p_Label, glm::vec3& p_Values, float p_ResetValue)
	{
		KTN_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();

		ImGui::PushID(p_Label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 100.0f); // TODO: Maybe change this?
		ImGui::Text(p_Label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		if (ImGui::Button("X", buttonSize))
			p_Values.x = p_ResetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &p_Values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		if (ImGui::Button("Y", buttonSize))
			p_Values.y = p_ResetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &p_Values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		if (ImGui::Button("Z", buttonSize))
			p_Values.z = p_ResetValue;
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &p_Values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	KTN_API void Tooltip(const char* p_Text)
	{
		KTN_PROFILE_FUNCTION();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(p_Text);
			ImGui::EndTooltip();
		}

		ImGui::PopStyleVar();
	}

	bool Combo(const char* p_Label, const char* p_PreviewText, const char* p_Items[], int p_ItemsCount, int* p_CurrentItem, float p_Spacing)
	{
		bool valueChanged = false;

		ImGui::BeginGroup();

		ImGui::Text("%s", p_Label);

		ImGui::SameLine(0.0f, p_Spacing);

		std::string comboLabel = "##Combo" + std::string(p_Label);

		if (ImGui::BeginCombo(comboLabel.c_str(), p_PreviewText)) 
		{
			for (int i = 0; i < p_ItemsCount; i++) 
			{
				bool isSelected = (*p_CurrentItem == i);
				if (ImGui::Selectable(p_Items[i], isSelected)) 
				{
					*p_CurrentItem = i;
					valueChanged = true;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::EndGroup();
		return valueChanged;
	}
} // namespace KTN::UI
