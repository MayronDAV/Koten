#pragma once
#include "Koten/Core/Base.h"

// lib
#include <glm/glm.hpp>



namespace KTN::Math
{
    class KTN_API BoundingSphere
    {
        public:
            BoundingSphere() = default;
            BoundingSphere(const glm::vec3& p_Center, float p_Radius) : m_Center(p_Center), m_Radius(p_Radius) {}
            BoundingSphere(const glm::vec3* p_Points, uint32_t p_Count);
            BoundingSphere(const glm::vec3* p_Points, uint32_t p_Count, const glm::vec3& p_Center);
            BoundingSphere(const glm::vec3* p_Points, uint32_t p_Count, const glm::vec3& p_Center, float p_ExtraRadius);
            BoundingSphere(const BoundingSphere& p_Other) : m_Center(p_Other.m_Center), m_Radius(p_Other.m_Radius) {}
            BoundingSphere(const glm::mat4& p_Transform);
            ~BoundingSphere() = default;

            bool IsInside(const glm::vec3& p_Point) const;
            bool IsInside(const BoundingSphere& p_Sphere) const;

            bool Contains(const glm::vec3& p_Point) const;
            bool Contains(const BoundingSphere& p_Other) const;
            bool Intersects(const BoundingSphere& p_Other) const;
            bool Intersects(const glm::vec3& p_Point) const;
            bool Intersects(const glm::vec3& p_Point, float p_Radius) const;

            void Merge(const BoundingSphere& p_Other);
            void Merge(const glm::vec3& p_Point);
            void Merge(const glm::vec3* p_Points, uint32_t p_Count);

            void Transform(const glm::mat4& p_Transform);
            BoundingSphere Transformed(const glm::mat4& p_Transform) const;

            void SetCenter(const glm::vec3& p_Center) { m_Center = p_Center; }
            void SetRadius(float p_Radius) { m_Radius = p_Radius; }
            void Set(const glm::vec3& p_Center, float p_Radius) { m_Center = p_Center; m_Radius = p_Radius; }

            const glm::vec3& GetCenter() const { return m_Center; }
            float GetRadius() const { return m_Radius; }

            BoundingSphere& operator=(const BoundingSphere& p_Other);

            bool IsValid() const { return m_Radius > 0.0f; }

        private:
            glm::vec3 m_Center{ 0.0f };
            float m_Radius{ 0.0f };
    };

} //namespace KTN::Math