#include "ktnpch.h"
#include "Frustum.h"


namespace KTN::Math
{
    void Frustum::Transform(const glm::mat4& p_Transform)
    {
        KTN_PROFILE_FUNCTION();

        for (auto& plane : m_Planes)
        {
            plane.Transform(p_Transform);
            plane.Normalize();
        }
    }

    void Frustum::Define(const glm::mat4& p_Transform)
    {
        KTN_PROFILE_FUNCTION();
        
        const glm::mat4& m = p_Transform;
        //                                                   Normal                                  Distance
        m_Planes[0]        = Plane({ m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0] }, m[3][3] + m[3][0]); // Left
        m_Planes[1]        = Plane({ m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0] }, m[3][3] - m[3][0]); // Right
        m_Planes[2]        = Plane({ m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1] }, m[3][3] + m[3][1]); // Bottom
        m_Planes[3]        = Plane({ m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1] }, m[3][3] - m[3][1]); // Top
        m_Planes[4]        = Plane({ m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2] }, m[3][3] + m[3][2]); // Near
        m_Planes[5]        = Plane({ m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2] }, m[3][3] - m[3][2]); // Far

        for (auto& plane : m_Planes)
        {
            plane.Normalize();
        }
    }

    void Frustum::Define(const glm::mat4& p_Projection, const glm::mat4& p_View)
    {
        KTN_PROFILE_FUNCTION();

        glm::mat4 m(p_Projection * p_View);
        Define(m);
    }

    void Frustum::DefineOrtho(float p_Scale, float p_AspectRatio, float p_N, float p_F, const glm::mat4& p_ViewMatrix)
    {
        KTN_PROFILE_FUNCTION();

        Define(glm::ortho(-p_Scale * p_AspectRatio * 0.5f, p_Scale * p_AspectRatio * 0.5f, -p_Scale * 0.5f, p_Scale * 0.5f, p_N, p_F), p_ViewMatrix);
    }

    void Frustum::DefinePerspective(float p_Fov, float p_AspectRatio, float p_N, float p_F, const glm::mat4& p_ViewMatrix)
    {
        KTN_PROFILE_FUNCTION();

        Define(glm::perspective(glm::radians(p_Fov), p_AspectRatio, p_N, p_F), p_ViewMatrix);
    }

    bool Frustum::IsInside(const glm::vec3& p_Point) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        for (const auto& plane : m_Planes)
        {
            if (!plane.IsOnPlane(p_Point))
                return false;
        }

        return true;
    }

    bool Frustum::IsInside(const BoundingSphere& p_Sphere) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        for (const auto& plane : m_Planes)
        {
            if (!plane.IsOnPlane(p_Sphere))
                return false;
        }

        return true;
    }

    bool Frustum::IsInside(const BoundingBox& p_Box) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        for (const auto& plane : m_Planes)
        {
            if (!plane.IsOnPlane(p_Box))
                return false;
        }

        return true;
    }

    Intersection Frustum::Intersects(const BoundingSphere& p_Sphere) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        bool intersecting = false;
        for (const Plane& plane : m_Planes)
        {
            const Intersection result = plane.Intersects(p_Sphere);

            if (result == Intersection::Outside)
                return Intersection::Outside;
            if (result == Intersection::Intersecting)
                intersecting = true;
        }

        return intersecting ? Intersection::Intersecting : Intersection::Inside;
    }

    Intersection Frustum::Intersects(const BoundingBox& p_Box) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        bool intersecting = false;
        for (const Plane& plane : m_Planes)
        {
            const Intersection result = plane.Intersects(p_Box);

            if (result == Intersection::Outside)
                return Intersection::Outside;
            if (result == Intersection::Intersecting)
                intersecting = true;
        }

        return intersecting ? Intersection::Intersecting : Intersection::Inside;
    }

} // namespace KTN::Math
