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
	KTN_API bool InputText(std::string& p_Text, const char* p_ID);
	KTN_API void DrawItemActivityOutline(float p_Rounding, bool p_DrawWhenInactive, ImColor p_ColorWhenActive = ImColor(80, 80, 80));
	KTN_API bool DrawColorEdit4(const std::string& p_Label, glm::vec4& p_Values, float p_ResetValue);
	KTN_API void DrawFloat3(const std::string& p_Label, glm::vec3& p_Values, float p_ResetValue = 0.0f);
	KTN_API void Tooltip(const char* p_Text);

} // namespace KTN::UI
