#include "test.h"

// std
#include <iostream>



namespace KTN
{
	KTN_API void Print(const char* p_Msg)
	{
		std::cout << p_Msg << "\n";
	}

} // namespace KTN
