#pragma once
#include "Base.h"

// std
#include <string>


namespace KTN
{
	class KTN_API FileSystem
	{
		public:
			static std::string GetName(const std::string& p_Path);
			static std::string GetStem(const std::string& p_Path);
			static std::string GetExtension(const std::string& p_Path);

			static std::string ReadFile(const std::string& p_Path);

			static void CreateDirectories(const std::string& p_Path);
	};

} // namespace KTN