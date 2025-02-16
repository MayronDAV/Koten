#include "ktnpch.h"
#include "UI.h"

#include "Platform/OpenGL/GLTexture.h"



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
} // namespace KTN::UI
