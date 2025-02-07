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

} // namespace KTN::UI
