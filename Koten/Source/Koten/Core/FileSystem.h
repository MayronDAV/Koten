#pragma once
#include "Base.h"

// std
#include <string>
#include <filesystem>
#include <vector>


namespace KTN
{
	class KTN_API FileSystem
	{
		public:
			static std::string GetName(const std::string& p_Path);
			static std::string GetStem(const std::string& p_Path);
			static std::string GetExtension(const std::string& p_Path);

			static bool WriteFile(const std::string& p_Path, uint8_t* p_Buffer, uint32_t p_Size);
			static bool WriteTextFile(const std::string& p_Path, const std::string& p_Text);
			static std::string ReadFile(const std::string& p_Path);

			static void Search(const std::string& p_Query, const std::string& p_Dir, std::vector<std::filesystem::path>& p_Results);

			static bool Exists(const std::string& p_Path);

			static void CreateDirectories(const std::string& p_Path);
	};

} // namespace KTN