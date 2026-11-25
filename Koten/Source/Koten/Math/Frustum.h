#pragma once
#include "Koten/Core/Base.h"
#include "BoundingSphere.h"
#include "Plane.h"

// lib
#include <glm/glm.hpp>

// std
#include <array>



namespace KTN::Math
{
    enum class FrustumPlane : uint8_t
    {
        Left = 0,
        Right,
        Bottom,
        Top,
        Near,
        Far
    };

    class KTN_API Frustum
    {
        public:
            Frustum() { Define({ 1.0f }); }
            Frustum(const glm::mat4& p_Transform) { Define(p_Transform); }
            Frustum(const glm::mat4& p_Projection, const glm::mat4& p_View) { Define(p_Projection, p_View); }
            ~Frustum() = default;

            void Transform(const glm::mat4& p_Transform);

            void Define(const glm::mat4& p_Transform);
            void Define(const glm::mat4& p_Projection, const glm::mat4& p_View);
            void DefineOrtho(float p_Scale, float p_AspectRatio, float p_N, float p_F, const glm::mat4& p_ViewMatrix);
            void DefinePerspective(float p_Fov, float p_AspectRatio, float p_N, float p_F, const glm::mat4& p_ViewMatrix);

            bool IsInside(const glm::vec3& p_Point) const;
            bool IsInside(const BoundingSphere& p_Sphere) const;
            bool IsInside(const BoundingBox& p_Box) const;

            Intersection Intersects(const BoundingSphere& p_Sphere) const;
            Intersection Intersects(const BoundingBox& p_Box) const;

            const Plane& GetPlane(FrustumPlane p_Plane) const { return m_Planes[(int)p_Plane]; }
            const Plane& GetPlane(uint32_t p_Index) const { return m_Planes[p_Index]; }

        private:
            std::array<Plane, 6> m_Planes;
    };

} // namespace KTN::Math