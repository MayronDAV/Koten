#include "ktnpch.h"
#include "BoundingBox.h"
#include "BoundingSphere.h"


namespace KTN::Math
{
    BoundingBox::BoundingBox(const glm::mat4& p_Transform)
    {
        KTN_PROFILE_FUNCTION_LOW();

        glm::vec3 worldCenter  = p_Transform[3];
        const glm::vec3 scale  = {
            glm::length(glm::vec3(p_Transform[0])),
            glm::length(glm::vec3(p_Transform[1])),
            glm::length(glm::vec3(p_Transform[2]))
        };
        glm::vec3 worldExtents = scale * 0.5f;

        m_Min                  = worldCenter - worldExtents;
        m_Max                  = worldCenter + worldExtents;
    }

    bool BoundingBox::Contains(const BoundingBox& p_Box) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!p_Box.IsValid())
            return false;

        return (p_Box.m_Min.x >= m_Min.x && p_Box.m_Min.y >= m_Min.y && p_Box.m_Min.z >= m_Min.z) &&
               (p_Box.m_Max.x <= m_Max.x && p_Box.m_Max.y <= m_Max.y && p_Box.m_Max.z <= m_Max.z);
    }

    bool BoundingBox::IsInside(const BoundingBox& p_Box) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!p_Box.IsValid())
            return false;

        if (p_Box.m_Max.x < m_Min.x || p_Box.m_Min.x > m_Max.x || p_Box.m_Max.y < m_Min.y || p_Box.m_Min.y > m_Max.y || p_Box.m_Max.z < m_Min.z || p_Box.m_Min.z > m_Max.z)
            return false;
        else
            return true;
    }

    bool BoundingBox::IsInside(const glm::vec3& p_Point) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (p_Point.x < m_Min.x || p_Point.x > m_Max.x || p_Point.y < m_Min.y || p_Point.y > m_Max.y || p_Point.z < m_Min.z || p_Point.z > m_Max.z)
            return false;
        else
            return true;
    }

    void BoundingBox::Merge(const BoundingBox& p_Other)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Other.IsValid())
            return;

        if (!IsValid())
        {
            m_Min = p_Other.m_Min;
            m_Max = p_Other.m_Max;
            return;
        }

        m_Min = glm::min(m_Min, p_Other.m_Min);
        m_Max = glm::max(m_Max, p_Other.m_Max);
    }

    void BoundingBox::Merge(const BoundingSphere& p_Sphere)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Sphere.IsValid())
            return;

        const glm::vec3 center = p_Sphere.GetCenter();
        const glm::vec3 extent(p_Sphere.GetRadius());

        const glm::vec3 min    = center - extent;
        const glm::vec3 max    = center + extent;

        if (!IsValid())
        {
            m_Min = min;
            m_Max = max;
            return;
        }

        m_Min = glm::min(m_Min, min);
        m_Max = glm::max(m_Max, max);
    }

    void BoundingBox::Merge(const glm::vec3& p_Point)
    {
        KTN_PROFILE_FUNCTION();

        m_Min = glm::min(m_Min, p_Point);
        m_Max = glm::max(m_Max, p_Point);
    }

    void BoundingBox::Merge(const glm::vec3* p_Points, uint32_t p_Count)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Points || p_Count == 0)
            return;

        for (uint32_t i = 0; i < p_Count; i++)
        {
            m_Min = glm::min(m_Min, p_Points[i]);
            m_Max = glm::max(m_Max, p_Points[i]);
        }
    }

    void BoundingBox::Transform(const glm::mat4& p_Transform)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!IsValid())
        {
            m_Min = glm::vec3(0.0f);
            m_Max = glm::vec3(0.0f);
        }

        glm::vec3 center       = GetCenter();
        glm::vec3 extents      = GetExtents();

        glm::mat3 rotationScale(p_Transform);
        glm::vec3 worldCenter  = p_Transform * glm::vec4(center, 1.0f);
        glm::vec3 worldExtents = rotationScale * glm::vec4(extents, 1.0f);

        m_Min                  = worldCenter - worldExtents;
        m_Max                  = worldCenter + worldExtents;
    }

    BoundingBox BoundingBox::Transformed(const glm::mat4& p_Transform) const
    {
        KTN_PROFILE_FUNCTION_LOW();

        BoundingBox box = *this;
        box.Transform(p_Transform);
        return box;
    }

    void BoundingBox::ExtendToCube()
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!IsValid()) return;

        glm::vec3 extents      = GetExtents();
        float maxDimension     = glm::max(glm::max(extents.x, extents.y), extents.z);
        glm::vec3 extentAmount = glm::vec3(maxDimension - extents.x, maxDimension - extents.y, maxDimension - extents.z) * 0.5f;

        m_Min                 -= extentAmount;
        m_Max                 += extentAmount;
    }

    BoundingBox& BoundingBox::operator=(const BoundingBox& p_Other)
    {
        KTN_PROFILE_FUNCTION_LOW();

        m_Min = p_Other.m_Min;
        m_Max = p_Other.m_Max;
        return *this;
    }

} // namespace KTN::Math
