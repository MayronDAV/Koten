#include "EditorCamera.h"
#include "Koten/Core/Time.h"
#include "Koten/OS/Input.h"
#include "Koten/OS/KeyCodes.h"
#include "Koten/OS/MouseCodes.h"
#include "Koten/Events/MouseEvent.h"



namespace KTN
{
	EditorCamera::EditorCamera()
		: m_Transform({ 0.0f, 0.0f, 1.0f })
	{
	}

	EditorCamera::~EditorCamera()
	{
	}

	void EditorCamera::OnUpdate()
	{
		KTN_PROFILE_FUNCTION();

		m_Camera.OnUpdate();

		if (m_HandleEvents)
		{
			if (Input::IsMouseButtonPressed(Mouse::Button_Right))
			{
				Input::SetCursorMode(CursorMode::Disabled);

				HandleMouse();

				HandleKeyboard();
			}
			else
				Input::SetCursorMode(CursorMode::Normal);
		}
	}

	void EditorCamera::SetViewportSize(uint32_t p_Width, uint32_t p_Height)
	{
		KTN_PROFILE_FUNCTION();

		m_Camera.SetViewportSize(p_Width, p_Height);
	}

	void EditorCamera::HandleKeyboard()
	{
		KTN_PROFILE_FUNCTION();

		float dt = (float)Time::GetDeltaTime();
		glm::vec3 front = m_Transform.GetLocalFrontDirection();
		glm::vec3 up = m_Transform.GetLocalUpDirection();
		glm::vec3 right = m_Transform.GetLocalRightDirection();

		glm::vec3 dir{ 0.0f, 0.0f, 0.0f };

		if (m_Mode == EditorCameraMode::TWODIM)
		{
			if (Input::IsKeyPressed(Key::A))
				dir += right * -1.0f;
			if (Input::IsKeyPressed(Key::D))
				dir +=  right;

			if (Input::IsKeyPressed(Key::W))
				dir += up;
			if (Input::IsKeyPressed(Key::S))
				dir += up * -1.0f;

			if (glm::length(dir) > 0.0f)
				dir = glm::normalize(dir);

			glm::vec3 position = m_Transform.GetLocalTranslation();
			position += dir * m_Speed * dt;
			m_Transform.SetLocalTranslation(position);
		}

		if (m_Mode == EditorCameraMode::FLYCAM)
		{
			if (Input::IsKeyPressed(Key::A))
				dir += right * -1.0f;
			if (Input::IsKeyPressed(Key::D))
				dir += right;

			if (Input::IsKeyPressed(Key::W))
				dir += front;
			if (Input::IsKeyPressed(Key::S))
				dir += front * -1.0f;

			glm::vec3 position = m_Transform.GetLocalTranslation();
			position += dir * m_Speed * dt;
			m_Transform.SetLocalTranslation(position);
		}
	}

	void EditorCamera::HandleMouse()
	{
		KTN_PROFILE_FUNCTION();

		float dt = (float)Time::GetDeltaTime();

		glm::vec2 mousePos = Input::GetMousePosition();

		static bool firstClick = true;
		if (Input::IsMouseButtonPressed(Mouse::Button_Right))
		{
			if (firstClick)
			{
				m_LastMousePos = mousePos;
				firstClick = false;
			}

			glm::vec2 delta = mousePos - m_LastMousePos;
			m_LastMousePos = mousePos;

			if (m_Mode == EditorCameraMode::FLYCAM)
			{
				const float sign = m_Transform.GetLocalUpDirection().y < 0.0f ? 1.0f : -1.0f; // Maybe change this?
				float yaw = glm::degrees(m_Transform.GetLocalRotation().y);
				float pitch = glm::degrees(m_Transform.GetLocalRotation().x);
				yaw += sign * delta.x * m_Sensitivity;
				pitch += sign * delta.y * m_Sensitivity;

				m_Transform.SetLocalRotation(glm::radians(glm::vec3{ pitch, yaw, 0.0f }));
			}
		}
		else
		{
			firstClick = false;
		}
	}

} // namespace KTN
