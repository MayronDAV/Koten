#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"
#include "Koten/Events/Event.h"

// lib
#include <glm/glm.hpp>

// std
#include <vector>



namespace KTN
{
	enum class ButtonState : uint8_t
	{
		None = 0,
		Pressed,
		Held,
		Released
	};

	struct ControllerButtonData
	{
		int Button;
		ButtonState State = ButtonState::None;
		ButtonState PrevState = ButtonState::None;
	};

	#define MAX_CONTROLLER_COUNT 16
	struct Controller
	{
		int ID;
		std::string Name;
		bool Present = false;
		std::array<ControllerButtonData, 64> Buttons;
		std::array<float, 16> Axes;
	};

	class KTN_API Input
	{
		public:
			static void SetCursorMode(CursorMode p_Mode);

			static bool IsKeyPressed(int p_Key);

			static bool IsKeyJustPressed(int p_Key);
			static bool IsKeyJustReleased(int p_Key);
			static bool IsKeyJustHeld(int p_Key);

			static bool IsMouseButtonPressed(int p_Button);
			static bool IsMouseButtonReleased(int p_Button);

			static glm::vec2 GetMousePosition();
			static float GetMouseX();
			static float GetMouseY();

			static int GetKeyPressed();

			static bool IsControllerPresent(int p_ID);
			static bool IsControllerButtonPressed(int p_ID, int p_Code);
			static bool IsControllerButtonHeld(int p_ID, int p_Code);
			static bool IsControllerButtonReleased(int p_ID, int p_Code);
			static float GetControllerAxis(int p_ID, int p_Code);
			static ButtonState GetControllerButtonState(int p_ID, int p_Code);

			static Controller* GetController(int p_ID);
			static std::vector<int> GetConnectedControllerIDs();

		private:
			static std::array<Controller, MAX_CONTROLLER_COUNT> s_Controllers;
	};


} // namespace KTN