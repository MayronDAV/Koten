#include "ktnpch.h"
#include "AABB.h"


namespace KTN
{
	AABB::AABB()
		: Min(0.0f), Max(0.0f)
	{
	}

	AABB::AABB(const glm::vec2& p_Min, const glm::vec2& p_Max)
	{
		Set(p_Min, p_Max);
	}

	AABB::AABB(const glm::vec2* p_Points, uint32_t p_NumPoints)
	{
		Set(p_Points, p_NumPoints);
	}

	void AABB::Set(const glm::vec2& p_Min, const glm::vec2& p_Max)
	{
		Min = p_Min;
		Max = p_Max;
	}

	void AABB::Set(const glm::vec2* p_Points, uint32_t p_NumPoints)
	{
		KTN_PROFILE_FUNCTION();

		Min = glm::vec2(0.0f);
		Max = glm::vec2(0.0f);

		for (uint32_t i = 0; i < p_NumPoints; i++)
		{
			Min = glm::min(Min, p_Points[i]);
			Max = glm::max(Max, p_Points[i]);
		}
	}

	void AABB::Merge(const glm::vec2& p_Point)
	{
		Min = glm::min(p_Point, Min);
		Max = glm::max(p_Point, Max);
	}

	float AABB::Area() const
	{
		return (Max.x - Min.x) * (Max.y - Min.y);
	}

	float AABB::Perimeter() const
	{
		glm::vec2 d = Max - Min;
		return 2.0f * (d.x + d.y);
	}

	bool AABB::Overlaps(const glm::vec2& p_Point) const
	{
		KTN_PROFILE_FUNCTION();

		if (Max.x < p_Point.x || Min.x > p_Point.x) return false;
		if (Max.y < p_Point.y || Min.y > p_Point.y) return false;
		return true;
	}

	bool AABB::Contains(const AABB& p_Other) const
	{
		return Min.x <= p_Other.Min.x && Min.y <= p_Other.Min.y && Max.x >= p_Other.Max.x && Max.y >= p_Other.Max.y;
	}

	bool AABB::Overlaps(const AABB& p_Other) const
	{
		KTN_PROFILE_FUNCTION();

		if (Max.x < p_Other.Min.x || Min.x > p_Other.Max.x) return false;
		if (Max.y < p_Other.Min.y || Min.y > p_Other.Max.y) return false;
		return true;
	}

	void AABB::Expand(float p_Margin)
	{
		Min -= glm::vec2(p_Margin);
		Max += glm::vec2(p_Margin);
	}

	AABB AABB::Union(const AABB& p_AABB, const glm::vec2& p_Point)
	{
		glm::vec2 min = glm::min(p_AABB.Min, p_Point);
		glm::vec2 max = glm::max(p_AABB.Max, p_Point);

		return AABB{ min, max };
	}

	AABB AABB::Union(const AABB& p_Lhs, const AABB& p_Rhs)
	{
		glm::vec2 min = glm::min(p_Lhs.Min, p_Rhs.Min);
		glm::vec2 max = glm::max(p_Lhs.Max, p_Rhs.Max);

		return AABB{ min, max };
	}

	AABB AABB::Intersection(const AABB& p_Lhs, const AABB& p_Rhs)
	{
		glm::vec2 min = glm::max(p_Lhs.Min, p_Rhs.Min);
		glm::vec2 max = glm::min(p_Lhs.Max, p_Rhs.Max);

		return AABB{ min, max };
	}


} // namespace KTN