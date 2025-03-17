#pragma once

#include "Koten/Core/Base.h"

// std
#include <string>
#include <vector>


namespace KTN
{
	class KTN_API StringUtils
	{
		public:
			static std::string Trim(const std::string& p_String, const std::string& p_CharsToRemove = " \t\n\r");
			static std::vector<std::string> Split(const std::string& p_String, const std::string& p_Delimiter);
			static std::vector<std::string> GetLines(const std::string& p_String);
	};
	
} // namespace KTN