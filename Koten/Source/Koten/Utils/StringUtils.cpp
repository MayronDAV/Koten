#include "ktnpch.h"
#include "StringUtils.h"



namespace KTN
{
	std::string StringUtils::Trim(const std::string& p_String, const std::string& p_CharsToRemove)
	{
		KTN_PROFILE_FUNCTION();

		size_t start = p_String.find_first_not_of(p_CharsToRemove);
		if (start == std::string::npos)
			return "";

		size_t end = p_String.find_last_not_of(p_CharsToRemove);

		return p_String.substr(start, end - start + 1);

	}
	std::vector<std::string> KTN::StringUtils::Split(const std::string& p_String, const std::string& p_Delimiter)
	{
		KTN_PROFILE_FUNCTION();

		size_t start = 0;
		size_t end = p_String.find_first_of(p_Delimiter);

		std::vector<std::string> result;

		while (end <= std::string::npos)
		{
			std::string token = p_String.substr(start, end - start);
			if (!token.empty())
				result.push_back(token);

			if (end == std::string::npos)
				break;

			start = end + 1;
			end = p_String.find_first_of(p_Delimiter, start);
		}

		return result;
	}

	std::vector<std::string> StringUtils::GetLines(const std::string& p_String)
	{
		KTN_PROFILE_FUNCTION();

		return Split(p_String, "\n");
	}

} // namespace KTN
