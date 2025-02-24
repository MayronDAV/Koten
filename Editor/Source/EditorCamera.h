#pragma once
#include "Koten/Graphics/Camera.h"
#include "Koten/Math/Transform.h"
#include "Koten/Events/Event.h"



namespace KTN
{
	enum class EditorCameraMode
	{
		TWODIM
	};

	class EditorCamera
	{
		public:
			EditorCamera();
			~EditorCamera();

			void OnUpdate();
			void SetViewportSize(uint32_t p_Width, uint32_t p_Height);

			void SetMode(EditorCameraMode p_Mode) { m_Mode = p_Mode; m_Camera.SetIsOrthographic(p_Mode == EditorCameraMode::TWODIM); }
			void SetSpeed(float p_Speed) { m_Speed = p_Speed; }
			void SetHandleEvents(bool p_Value) { m_HandleEvents = p_Value; }

			float GetSpeed() const { return m_Speed; }
			Camera& GetCamera() { return m_Camera; }
			EditorCameraMode GetMode() const { return m_Mode; }
			const glm::mat4& GetProjection() const { return m_Camera.GetProjection(); }
			const glm::mat4& GetView() const { return m_Transform.GetLocalMatrix() == glm::mat4(1.0f) ? glm::mat4(1.0f) : glm::inverse(m_Transform.GetLocalMatrix()); }
			Math::Transform& GetTransform() { return m_Transform; }

		private:
			void HandleKeyboard();

		private:
			Camera m_Camera{ true, 800, 600 };
			EditorCameraMode m_Mode = EditorCameraMode::TWODIM;
			float m_Speed = 5.0f;

			Math::Transform m_Transform;

			bool m_HandleEvents = true;
	};


} // namespace KTN