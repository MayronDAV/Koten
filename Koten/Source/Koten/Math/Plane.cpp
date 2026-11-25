#include "ktnpch.h"
#include "Plane.h"



namespace KTN::Math
{
    void Plane::Normalize()
    {
        KTN_PROFILE_FUNCTION();

        float length = glm::length(m_Normal);
        if (length > 0.0f)
        {
            m_Normal       /= length;
            m_Distance     /= length;
        }
    }

    float Plane::Distance(const glm::vec3& p_Point) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        return glm::dot(p_Point, m_Normal) + m_Distance;
    }

    bool Plane::IsOnPlane(const glm::vec3& p_Point) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        return Distance(p_Point) >= -MATH_EPSILON;
    }

    bool Plane::IsOnPlane(const BoundingSphere& p_Sphere) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        return Distance(p_Sphere.GetCenter()) >= -p_Sphere.GetRadius();
    }

    bool Plane::IsOnPlane(const BoundingBox& p_Box) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        float radius   = glm::dot(glm::abs(m_Normal), p_Box.GetExtents());
        return Distance(p_Box.GetCenter()) + radius >= -MATH_EPSILON;
    }

    Intersection Plane::Intersects(const glm::vec3& p_Point) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        return Distance(p_Point) >= -MATH_EPSILON ? Intersection::Inside : Intersection::Outside;
    }

    Intersection Plane::Intersects(const BoundingSphere& p_Sphere) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        const float distance = Distance(p_Sphere.GetCenter());
        const float radius   = p_Sphere.GetRadius();

        if (distance + radius < -MATH_EPSILON)
            return Intersection::Outside;

        if (distance - radius < -MATH_EPSILON)
            return Intersection::Intersecting;

        return Intersection::Inside;
    }

    Intersection Plane::Intersects(const BoundingBox& p_Box) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        const float distance = Distance(p_Box.GetCenter());
        const float radius   = glm::dot(glm::abs(m_Normal), p_Box.GetExtents());

        if (distance + radius < -MATH_EPSILON)
            return Intersection::Outside;

        if (distance - radius < -MATH_EPSILON)
            return Intersection::Intersecting;

        return Intersection::Inside;
    }

    void Plane::Transform(const glm::mat4& p_Transform)
    {
        KTN_PROFILE_FUNCTION();

        glm::vec4 plane(m_Normal, m_Distance);
        plane      = p_Transform * plane;
        m_Normal   = plane;
        m_Distance = plane.w;
    }

    Plane Plane::Transformed(const glm::mat4& p_Transform) const
    {
        KTN_PROFILE_FUNCTION();

        glm::vec4 plane(m_Normal, m_Distance);
        plane = p_Transform * plane;
        return Plane(plane);
    }

} // namespace KTN::Math
