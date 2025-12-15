#include "ktnpch.h"
#include "Intersect2D.h"

// lib
#include <glm/gtx/quaternion.hpp>



namespace KTN
{
	bool Intersect2D::IsColliding(const BoxShape& p_A, const BoxShape& p_B)
	{
        KTN_PROFILE_FUNCTION();

        glm::vec2 axes[4] =
        {
            {  cos(p_A.Rotation), sin(p_A.Rotation) },
            { -sin(p_A.Rotation), cos(p_A.Rotation) },
            {  cos(p_B.Rotation), sin(p_B.Rotation) },
            { -sin(p_B.Rotation), cos(p_B.Rotation) }
        };

        auto project = [](const BoxShape& box, const glm::vec2& axis)
        {
            glm::vec2 x = { cos(box.Rotation), sin(box.Rotation) };
            glm::vec2 y = { -x.y, x.x };

            float proj =
                box.HalfExtents.x * abs(glm::dot(axis, x)) +
                box.HalfExtents.y * abs(glm::dot(axis, y));

            float center = glm::dot(axis, box.Center);
            return std::pair<float, float>{ center - proj, center + proj };
        };

        for (int i = 0; i < 4; ++i)
        {
            auto [minA, maxA] = project(p_A, axes[i]);
            auto [minB, maxB] = project(p_B, axes[i]);

            if (maxA < minB || maxB < minA)
                return false;
        }

        return true;
	}

	bool Intersect2D::IsColliding(const CircleShape& p_A, const BoxShape& p_B)
	{
        KTN_PROFILE_FUNCTION();

        float c = cos(-p_B.Rotation);
        float s = sin(-p_B.Rotation);

        glm::vec2 local =
        {
            c * (p_A.Center.x - p_B.Center.x) -
            s * (p_A.Center.y - p_B.Center.y),
            s * (p_A.Center.x - p_B.Center.x) +
            c * (p_A.Center.y - p_B.Center.y)
        };

        glm::vec2 closest =
        {
            glm::clamp(local.x, -p_B.HalfExtents.x, p_B.HalfExtents.x),
            glm::clamp(local.y, -p_B.HalfExtents.y, p_B.HalfExtents.y)
        };

        glm::vec2 d = local - closest;
        return glm::dot(d, d) <= p_A.Radius * p_A.Radius;
	}

	bool Intersect2D::IsColliding(const CircleShape& p_A, const CircleShape& p_B)
	{
        KTN_PROFILE_FUNCTION();

        glm::vec2 d = p_B.Center - p_A.Center;
        float dist2 = glm::dot(d, d);
        float r = p_A.Radius + p_B.Radius;

        return dist2 <= r * r;
	}

} // namespace KTN
