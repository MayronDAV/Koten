#pragma once
#include "Koten/Core/Base.h"

// lib
#include <glm/glm.hpp>



namespace KTN::Math
{
    class KTN_API BoundingSphere;

    class KTN_API BoundingBox
    {
        public:
            BoundingBox() = default;
            BoundingBox(const glm::vec3& p_Min, const glm::vec3& p_Max) : m_Min(p_Min), m_Max(p_Max) {}
            BoundingBox(const BoundingBox& p_Other) : m_Min(p_Other.m_Min), m_Max(p_Other.m_Max) {}
            BoundingBox(const glm::vec3* p_Points, uint32_t p_Count) { Merge(p_Points, p_Count); }
            BoundingBox(const BoundingSphere& p_Sphere) { Merge(p_Sphere); }
            BoundingBox(const glm::mat4& p_Transform);
            ~BoundingBox() = default;

            bool Contains(const BoundingBox& p_Box) const;

            bool IsInside(const BoundingBox& p_Box) const;
            bool IsInside(const glm::vec3& p_Point) const;

            void Merge(const BoundingBox& p_Other);
            void Merge(const BoundingSphere& p_Sphere);
            void Merge(const glm::vec3& p_Point);
            void Merge(const glm::vec3* p_Points, uint32_t p_Count);

            void Transform(const glm::mat4& p_Transform);
            BoundingBox Transformed(const glm::mat4& p_Transform) const;

            void SetMin(const glm::vec3& p_Min) { m_Min = p_Min; }
            void SetMax(const glm::vec3& p_Max) { m_Max = p_Max; }
            void Set(const glm::vec3& p_Min, const glm::vec3& p_Max) { m_Min = p_Min; m_Max = p_Max; }

            glm::vec3 GetSize() const { return m_Max - m_Min; }
            glm::vec3 GetCenter() const { return (m_Max + m_Min) * 0.5f; }
            glm::vec3 GetMin() const { return m_Min; }
            glm::vec3 GetMax() const { return m_Max; }
            glm::vec3 GetExtents() const { return m_Max - m_Min; }
            float GetSurfaceArea() const
            {
                const glm::vec3 size = glm::max(GetSize(), glm::vec3(0.0f));
                return 2.0f * (size.x * size.y + size.y * size.z + size.z * size.x);
            }

            void ExtendToCube();

            bool IsCube() const { return m_Min.z != 0.0f || m_Max.z != 0.0f; }

            bool IsValid() const
            {
                return m_Min.x <= m_Max.x && m_Min.y <= m_Max.y && m_Min.z <= m_Max.z;
            }

            BoundingBox& operator=(const BoundingBox& p_Other);
            bool operator==(const BoundingBox& p_Other)
            {
                return p_Other.m_Min == m_Min && p_Other.m_Max == m_Max;
            }

            static BoundingBox Union(const BoundingBox& p_A, const BoundingBox& p_B)
            {
                BoundingBox result(p_A);
                result.Merge(p_B);
                return result;
            }

        private:
            glm::vec3 m_Min{ FLT_MAX };
            glm::vec3 m_Max{ -FLT_MAX };
    };

} // namespace KTN::Math