#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Scene/Entity.h"


namespace KTN
{
	class KTN_API DebugRenderer
	{
		public:
			static void DrawCollider2D(Entity p_Entity, const glm::vec4& p_Color);
			static void DrawSquare(const glm::mat4& p_Transform, const glm::vec4& p_Color, int p_Entity = -1);
			static void DrawCircle(const glm::mat4& p_Transform, const glm::vec4& p_Color, int p_Entity = -1);
	};

} // namespace KTN