#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"
#include "Koten/Events/Event.h"

// lib
#include <glm/glm.hpp>



namespace KTN
{
	class KTN_API Input
	{
		public:
			static void SetCursorMode(CursorMode p_Mode);

			static bool IsKeyPressed(int p_Key);
			static bool IsKeyReleased(int p_Key);

			static bool IsMouseButtonPressed(int p_Button);

			static glm::vec2 GetMousePosition();
			static float GetMouseX();
			static float GetMouseY();

			static int GetKeyPressed();
	};


} // namespace KTN