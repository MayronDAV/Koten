#pragma once

// std
#include <cstdint>



namespace KTN
{
	using MouseCode = uint16_t;

	// From glfw3.h
	namespace Mouse
	{
		enum : MouseCode
		{
			Button_1 = 0,
			Button_2 = 1,
			Button_3 = 2,
			Button_4 = 3,
			Button_5 = 4,
			Button_6 = 5,
			Button_7 = 6,
			Button_8 = 7,
			Button_Last = Button_8,
			Button_Left = Button_1,
			Button_Right = Button_2,
			Button_Middle = Button_3,
		};

	} // namespace Mouse

} // namespace KTN