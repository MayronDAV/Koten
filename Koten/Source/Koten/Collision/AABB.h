#pragma once
#include "Koten/Core/Base.h"

// lib
#include <glm/glm.hpp>



namespace KTN
{
	class KTN_API AABB
	{
		public:
			AABB();
			AABB(const glm::vec2& p_Min, const glm::vec2& p_Max);
			AABB(const glm::vec2* p_Points, uint32_t p_NumPoints);
			~AABB() = default;

			void Set(const glm::vec2& p_Min, const glm::vec2& p_Max);
			void Set(const glm::vec2* p_Points, uint32_t p_NumPoints);
			void Merge(const glm::vec2& p_Point);
			void Clear() { Min = glm::vec2(0.0f), Max = glm::vec2(0.0f); }

			float Area() const;
			float Perimeter() const;

			glm::vec2 Center() const { return (Min + Max) * 0.5f; }
			glm::vec2 Extents() const { return (Max - Min) * 0.5f; }
			float Width() const { return Max.x - Min.x; }
			float Height() const { return Max.y - Min.y; }

			bool Point(const glm::vec2& p_Point) const;
			bool Contains(const AABB& p_Other) const;
			bool Overlaps(const AABB& p_Other) const;
			void Expand(float p_Margin);

			glm::vec2 Min{ 0.0f };
			glm::vec2 Max{ 0.0f };

			static AABB Union(const AABB& p_AABB, const glm::vec2& p_Point);
			static AABB Union(const AABB& p_Lhs, const AABB& p_Rhs);
			static AABB Intersection(const AABB& p_Lhs, const AABB& p_Rhs);
	};


} // namespace KTN