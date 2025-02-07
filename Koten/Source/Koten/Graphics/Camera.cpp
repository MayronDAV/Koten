#include "ktnpch.h"
#include "Camera.h"



namespace KTN
{
	#define _KTN_SET_VALUE(value)	\
		if (p_Value == value)		\
			return;					\
		value = p_Value;			\
		m_ProjectionDirty = true;

	void Camera::SetIsOrthographic(bool p_Value)
	{
		_KTN_SET_VALUE(m_Orthographic)
	}

	void Camera::SetViewportSize(uint32_t p_Width, uint32_t p_Height)
	{
		if (m_FixAspectRatio && (m_ViewportWidth > 0 && m_ViewportHeight > 0))
			return;

		if ((p_Width > 0 && p_Height > 0) && (m_ViewportWidth != p_Width || m_ViewportHeight != p_Height))
		{
			m_ViewportWidth = p_Width;
			m_ViewportHeight = p_Height;
			m_ProjectionDirty = true;
		}
	}

	void Camera::SetFar(float p_Value)
	{
		_KTN_SET_VALUE(m_FarZ)
	}

	void Camera::SetNear(float p_Value)
	{
		_KTN_SET_VALUE(m_NearZ)
	}

	void Camera::SetScale(float p_Value)
	{
		_KTN_SET_VALUE(m_Scale)

		m_Scale = std::max(p_Value, 0.000001f);
	}

	void Camera::SetFOV(float p_Value)
	{
		_KTN_SET_VALUE(m_Fov)

		m_Fov = std::max(p_Value, 0.000001f);
	}

	void Camera::SetZoom(float p_Value)
	{
		_KTN_SET_VALUE(m_Zoom)

		m_Zoom = std::max(m_Zoom, 0.000001f);
		if (m_Orthographic)
		{
			m_Zoom = std::min(m_Zoom, 1.990f);
		}
	}

	void Camera::SetFixAspectRatio(bool p_Value)
	{
		_KTN_SET_VALUE(m_FixAspectRatio)
	}

	void Camera::OnUpdate()
	{
		if (!m_ProjectionDirty || (m_ViewportWidth <= 0 || m_ViewportHeight <= 0))
			return;

		float aspect = float(m_ViewportWidth) / float(m_ViewportHeight);
		if (m_Orthographic)
		{
			float scale			=  m_Scale * m_Zoom;
			float orthoLeft		= -scale * aspect * 0.5f;
			float orthoRight	=  scale * aspect * 0.5f;
			float orthoBottom	= -scale * 0.5f;
			float orthoTop		=  scale * 0.5f;

			m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_NearZ, m_FarZ);
		}
		else
			m_Projection = glm::perspective(glm::radians(m_Fov * m_Zoom), aspect, m_NearZ, m_FarZ);

		m_ProjectionDirty = false;
	}
} // namespace KTN
