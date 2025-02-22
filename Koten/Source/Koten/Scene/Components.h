#pragma once
#include "Koten/Math/Transform.h"
#include "Koten/Graphics/Texture.h"

// std
#include <string>



namespace KTN
{
	using TransformComponent = Math::Transform;

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& p_Tag)
			: Tag(p_Tag) {}
	};

	struct SpriteComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture = nullptr;

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;
		SpriteComponent(const glm::vec4& p_Color)
			: Color(p_Color) {}
		SpriteComponent(const glm::vec4& p_Color, const Ref<Texture2D>& p_Texture)
			: Color(p_Color), Texture(p_Texture) {}
	};

} // namespace KTN