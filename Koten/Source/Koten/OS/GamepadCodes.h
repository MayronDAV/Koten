#pragma once

// std
#include <cstdint>

namespace KTN
{
	using GamepadCode = uint8_t;
	using GamepadAxisCode = int;

	// From glfw3.h
	namespace Gamepad
	{
		enum : GamepadCode
		{
			ButtonSouth = 0, /* A (Xbox) or X (PS) */
			ButtonEast = 1, /* B (Xbox) or Circle (PS) */
			ButtonWest = 2, /* X (Xbox) or Square (PS) */
			ButtonNoth = 3, /* Y (Xbox) or Triangle (PS) */
			L1 = 4,
			R1 = 5,
			Back = 6, /* Select / Share */
			Start = 7, /* Start / Options */
			LeftThumb = 8,
			RightThumb = 9,
			Guide = 10, /* Xbox / PS button */
			DPadUp = 11,
			DPadRight = 12,
			DPadDown = 13,
			DPadLeft = 14
		};

		enum : GamepadAxisCode
		{
			AxisLeftX = 0,
			AxisLeftY = 1,
			AxisRightX = 2,
			AxisRightY = 3,
			AxisLeftTrigger = 4,
			AxisRightTrigger = 5
		};
	}

} // namespace KTN
