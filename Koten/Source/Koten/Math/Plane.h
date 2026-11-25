#pragma once
#include "Koten/Core/Base.h"
#include "BoundingSphere.h"
#include "BoundingBox.h"

// lib
#include <glm/glm.hpp>



namespace KTN::Math
{
    class KTN_API Plane
    {
        public:
            Plane() = default;
            Plane(const glm::vec3& p_Normal, float p_Distance) : m_Normal(p_Normal), m_Distance(p_Distance) {}
            Plane(const glm::vec4& p_Plane) : m_Normal(p_Plane), m_Distance(p_Plane.w) {}
            Plane(float p_X, float p_Y, float p_Z, float p_Distance) : m_Normal(p_X, p_Y, p_Z), m_Distance(p_Distance) {}
            ~Plane() = default;

            void SetNormal(const glm::vec3& p_Normal) { m_Normal = p_Normal; }
            void SetDistance(float p_Distance) { m_Distance = p_Distance; }
            void Set(const glm::vec3& p_Normal, float p_Distance) { m_Normal = p_Normal; m_Distance = p_Distance; }
            void Set(const glm::vec4& p_Plane) { m_Normal = p_Plane; m_Distance = p_Plane.w; }
            void Set(float p_X, float p_Y, float p_Z, float p_Distance) { m_Normal = { p_X, p_Y, p_Z }; m_Distance = p_Distance; }

            void Normalize();

            float Distance(const glm::vec3& p_Point) const;

            bool IsOnPlane(const glm::vec3& p_Point) const;
            bool IsOnPlane(const BoundingSphere& p_Sphere) const;
            bool IsOnPlane(const BoundingBox& p_Box) const;

            Intersection Intersects(const glm::vec3& p_Point) const;
            Intersection Intersects(const BoundingSphere& p_Sphere) const;
            Intersection Intersects(const BoundingBox& p_Box) const;

            glm::vec3 Project(const glm::vec3& p_Point) const { return p_Point - Distance(p_Point) * m_Normal; }

            void Transform(const glm::mat4& p_Transform);
            Plane Transformed(const glm::mat4& p_Transform) const;

            const glm::vec3& Normal() const { return m_Normal; }
            float Distance() const { return m_Distance; }

        private:
            glm::vec3 m_Normal{ 0.0f };
            float m_Distance{ 0.0f };
    };

} // namespace KTN::Math