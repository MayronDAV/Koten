#include "ktnpch.h"
#include "BoundingSphere.h"



namespace KTN::Math
{

    BoundingSphere::BoundingSphere(const glm::vec3* p_Points, uint32_t p_Count)
        : m_Center(0.0f), m_Radius(0.0f)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Count == 0)
            return;

        Merge(p_Points, p_Count);
    }

    BoundingSphere::BoundingSphere(const glm::vec3* p_Points, uint32_t p_Count, const glm::vec3& p_Center)
        : m_Center(p_Center), m_Radius(0.0f)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Count == 0)
            return;

        Merge(p_Points, p_Count);
    }

    BoundingSphere::BoundingSphere(const glm::vec3* p_Points, uint32_t p_Count, const glm::vec3& p_Center, float p_Radius)
        : m_Center(p_Center), m_Radius(p_Radius)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Count == 0)
            return;

        Merge(p_Points, p_Count);
    }

    BoundingSphere::BoundingSphere(const glm::mat4& p_Transform)
        : m_Center(p_Transform[3])
    {
        const glm::vec3 scale = {
            glm::length(glm::vec3(p_Transform[0])),
            glm::length(glm::vec3(p_Transform[1])),
            glm::length(glm::vec3(p_Transform[2]))
        };
        m_Radius = glm::length(scale) * 0.5f;
    }

    bool BoundingSphere::IsInside(const glm::vec3& p_Point) const
    {
        KTN_PROFILE_FUNCTION();

        glm::vec3 temp(p_Point - m_Center);
        return glm::dot(temp, temp) <= m_Radius * m_Radius;
    }

    bool BoundingSphere::IsInside(const BoundingSphere& p_Sphere) const
    {
        KTN_PROFILE_FUNCTION();

        glm::vec3 temp(p_Sphere.m_Center - m_Center);
        return glm::dot(temp, temp) <= (m_Radius + p_Sphere.m_Radius) * (m_Radius + p_Sphere.m_Radius);
    }

    bool BoundingSphere::Contains(const glm::vec3& p_Point) const
    {
        KTN_PROFILE_FUNCTION();

        glm::vec3 temp(p_Point - m_Center);
        return glm::dot(temp, temp) <= m_Radius * m_Radius;
    }

    bool BoundingSphere::Contains(const BoundingSphere& p_Other) const
    {
        KTN_PROFILE_FUNCTION();

        glm::vec3 temp(p_Other.m_Center - m_Center);
        return glm::dot(temp, temp) <= (m_Radius + p_Other.m_Radius) * (m_Radius + p_Other.m_Radius);
    }

    bool BoundingSphere::Intersects(const BoundingSphere& p_Other) const
    {
        KTN_PROFILE_FUNCTION();

        glm::vec3 temp(p_Other.m_Center - m_Center);
        return glm::dot(temp, temp) <= (m_Radius + p_Other.m_Radius) * (m_Radius + p_Other.m_Radius);
    }

    bool BoundingSphere::Intersects(const glm::vec3& p_Point) const
    {
        KTN_PROFILE_FUNCTION();

        glm::vec3 temp(p_Point - m_Center);
        return glm::dot(temp, temp) <= m_Radius * m_Radius;
    }

    bool BoundingSphere::Intersects(const glm::vec3& p_Point, float p_ExtraRadius) const
    {
        KTN_PROFILE_FUNCTION();

        glm::vec3 temp(p_Point - m_Center);
        return glm::dot(temp, temp) <= (m_Radius + p_ExtraRadius) * (m_Radius + p_ExtraRadius);
    }

    void BoundingSphere::Merge(const BoundingSphere& p_Other)
    {
        KTN_PROFILE_FUNCTION();

        float distance = glm::distance(p_Other.m_Center, m_Center);
        if (distance > m_Radius + p_Other.m_Radius)
            return;

        if (distance <= m_Radius - p_Other.m_Radius)
        {
            m_Center = p_Other.m_Center;
            m_Radius = p_Other.m_Radius;
            return;
        }

        if (distance <= p_Other.m_Radius - m_Radius)
            return;

        float half  = (distance + m_Radius + p_Other.m_Radius) * 0.5f;
        float scale = half / distance;
        m_Center    = (m_Center + p_Other.m_Center) * scale;
        m_Radius    = half;
    }

    void BoundingSphere::Merge(const glm::vec3& p_Point)
    {
        KTN_PROFILE_FUNCTION();

        float distance = glm::distance(p_Point, m_Center);
        if (distance > m_Radius)
            return;

        if (distance <= 0.0f)
        {
            m_Center = p_Point;
            m_Radius = 0.0f;
            return;
        }

        float half  = (distance + m_Radius) * 0.5f;
        float scale = half / distance;
        m_Center    = (m_Center + p_Point) * scale;
        m_Radius    = half;
    }

    void BoundingSphere::Merge(const glm::vec3* p_Points, uint32_t p_Count)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Count == 0 || !p_Points)
            return;

        float radius     = 0.0f;
        glm::vec3 center = p_Points[0];

        for (uint32_t i = 1; i < p_Count; i++)
        {
            float distance = glm::distance(p_Points[i], center);
            if (distance > radius)
                radius     = distance;

            center        += p_Points[i];
        }

        center        /= (float)p_Count;

        float distance = glm::distance(center, m_Center);
        if (distance > m_Radius)
            return;

        if (distance <= 0.0f)
        {
            m_Center = center;
            m_Radius = 0.0f;
            return;
        }

        float half  = (distance + m_Radius + radius) * 0.5f;
        float scale = half / distance;
        m_Center    = (m_Center + center) * scale;
        m_Radius    = half;
    }

    void BoundingSphere::Transform(const glm::mat4& p_Transform)
    {
        KTN_PROFILE_FUNCTION_LOW();

        m_Center = p_Transform * glm::vec4(m_Center, 1.0f);
    }

    BoundingSphere BoundingSphere::Transformed(const glm::mat4& p_Transform) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        glm::vec3 center = p_Transform * glm::vec4(m_Center, 1.0f);
        return BoundingSphere(center, m_Radius);
    }

    BoundingSphere& BoundingSphere::operator=(const BoundingSphere& p_Other)
    {
        KTN_PROFILE_FUNCTION_LOW();

        m_Center = p_Other.m_Center;
        m_Radius = p_Other.m_Radius;
        return *this;
    }

} // namespace KTN::Math