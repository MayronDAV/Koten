#pragma once
#include "Koten/Core/Base.h"



namespace KTN
{
	struct BoxShape
	{
		glm::vec2 Center;
		glm::vec2 HalfExtents;
		float Rotation; // rad
	};

	struct CircleShape
	{
		glm::vec2 Center;
		float Radius;
	};


	class KTN_API Intersect2D
	{
		public:
			static bool IsColliding(const BoxShape& p_A, const BoxShape& p_B);
			static bool IsColliding(const CircleShape& p_A, const BoxShape& p_B);
			static bool IsColliding(const CircleShape& p_A, const CircleShape& p_B);
	};

} // namespace KTN