#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Graphics/Texture.h"

// lib
#include <imgui.h>



namespace KTN::UI
{
	KTN_API ImTextureID AddImage(const Ref<Texture2D>& p_Texture);
	KTN_API void Image(const Ref<Texture2D>& p_Texture, const ImVec2& p_Size, const ImVec4& p_TintColor = { 1, 1, 1, 1 }, const ImVec4& p_BorderColor = { 0, 0, 0, 0 });
	KTN_API void Image(const Ref<Texture2D>& p_Texture, const ImVec2& p_Size, const ImVec2& p_UV0, const ImVec2& p_UV1, const ImVec4& p_TintColor = { 1, 1, 1, 1 }, const ImVec4& p_BorderColor = { 0, 0, 0, 0 });
	KTN_API void ImageButton(const Ref<Texture2D>& p_Texture, const ImVec2& p_Size);
	KTN_API void ImageButton(const Ref<Texture2D>& p_Texture, const ImVec2& p_Size, const ImVec2& p_UV0, const ImVec2& p_UV1);
	KTN_API bool InputText(const std::string& p_Label, std::string& p_Text, bool p_DrawLabel = false, ImGuiInputTextFlags p_Flags = 0, float p_OutlineRouding = 2.0f, bool p_DrawOutlineWhenInactive = false, ImColor p_OutlineColor = ImColor(80, 80, 80));
	KTN_API void DrawItemOutline(float p_Rounding, float p_Thickness, bool p_DrawWhenInactive, ImColor p_Color = ImColor(50, 50, 50), ImColor p_ColorHovered = ImColor(60, 60, 60), ImColor p_ColorActived = ImColor(80, 80, 80));
	KTN_API bool ColorEdit4(const std::string& p_Label, glm::vec4& p_Values, float p_ResetValue);
	KTN_API bool DragFloat3(const std::string& p_Label, glm::vec3& p_Values, float p_ResetValue = 0.0f);
	KTN_API bool InputFloat2(const std::string& p_Label, glm::vec2& p_Values, float p_ResetValue = 0.0f);
	KTN_API void Tooltip(const char* p_Text);
	KTN_API bool Combo(const char* p_Label, const char* p_PreviewText, const char* p_Items[], int p_ItemsCount, int* p_CurrentItem, float p_Spacing = -1.0f);

} // namespace KTN::UI
