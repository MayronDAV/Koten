#pragma once
#include "Koten/Core/Base.h"

// lib
#include <glm/glm.hpp>



namespace KTN
{
	class KTN_API Camera
	{
		public:
			Camera() = default;
			Camera(bool p_IsOrthographic)
				: m_Orthographic(p_IsOrthographic) { OnUpdate(); }
			Camera(bool p_IsOrthographic, uint32_t p_Width, uint32_t p_Height) 
				: m_Orthographic(p_IsOrthographic), m_ViewportWidth(p_Width), m_ViewportHeight(p_Width) { OnUpdate(); }
			Camera(uint32_t p_Width, uint32_t p_Height) 
				: m_ViewportWidth(p_Width), m_ViewportHeight(p_Width) { OnUpdate(); }
			~Camera() = default;

			void SetIsOrthographic(bool p_Value);
			void SetViewportSize(uint32_t p_Width, uint32_t p_Height);
			void SetFar(float p_Value);
			void SetNear(float p_Value);
			void SetScale(float p_Value);
			void SetFOV(float p_Value);
			void SetZoom(float p_Value);
			void SetFixAspectRatio(bool p_Value);

			float GetFar() const { return m_FarZ; }
			float GetNear() const { return m_NearZ; }
			float GetFOV() const { return m_Fov; }
			float GetScale() const { return m_Scale; }
			bool IsOrthographic() const { return m_Orthographic; }
			bool IsAspectRatioFixed() const { return m_FixAspectRatio; }
			uint32_t GetViewportWidth() const { return m_ViewportWidth; }
			uint32_t GetViewportHeight() const { return m_ViewportHeight; }
			float GetZoom() const { return m_Zoom; }
			const glm::mat4& GetProjection() const { return m_Projection; }

			void OnUpdate();

		private:
			glm::mat4 m_Projection	= glm::mat4(1.0f);
			bool m_ProjectionDirty	= true;

			uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
			float m_Scale			= 10.0f;
			float m_Zoom			= 1.0f;

			float m_Fov = 90.0f, m_NearZ = 0.001f, m_FarZ = 1000.0f;

			bool m_Orthographic		= false;
			bool m_FixAspectRatio	= false;
	};

} // namespace KTN
