#include "ktnpch.h"
#include "Input.h"

namespace KTN
{
	std::array<Controller, MAX_CONTROLLER_COUNT> Input::s_Controllers;

	bool Input::IsControllerPresent(int p_ID)
	{
		if (p_ID >= (int)s_Controllers.size())
			return false;

		return s_Controllers[p_ID].Present;
	}

	bool Input::IsControllerButtonPressed(int p_ID, int p_Code)
	{
		return GetControllerButtonState(p_ID, p_Code) == ButtonState::Pressed;
	}

	bool Input::IsControllerButtonHeld(int p_ID, int p_Code)
	{
		return GetControllerButtonState(p_ID, p_Code) == ButtonState::Held;
	}

	bool Input::IsControllerButtonReleased(int p_ID, int p_Code)
	{
		return GetControllerButtonState(p_ID, p_Code) == ButtonState::Released;
	}

	float Input::GetControllerAxis(int p_ID, int p_Code)
	{
		if (p_ID >= (int)s_Controllers.size() || p_Code >= 16)
			return false;

		return s_Controllers[p_ID].Axes[p_Code];
	}

	ButtonState Input::GetControllerButtonState(int p_ID, int p_Code)
	{
		if (p_ID >= (int)s_Controllers.size() || p_Code >= 64)
			return ButtonState::None;

		return s_Controllers[p_ID].Buttons[p_Code].State;
	}

	Controller* Input::GetController(int p_ID)
	{
		if (p_ID >= (int)s_Controllers.size())
			return nullptr;

		return &s_Controllers[p_ID];
	}

	std::vector<int> Input::GetConnectedControllerIDs()
	{
		std::vector<int> ids;
		ids.reserve(MAX_CONTROLLER_COUNT);
		for (int i = 0; i < MAX_CONTROLLER_COUNT; i++)
		{
			if (s_Controllers[i].Present)
			{
				ids.push_back(i);
			}
		}

		return ids;
	}

} // namespace KTN