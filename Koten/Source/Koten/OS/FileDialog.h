#pragma once
#include "Koten/Core/Base.h"

// std
#include <string>
#include <vector>



namespace KTN
{
	enum class FileDialogResult
	{
		SUCCESS,
		FAILED,
		CANCEL
	};

	class KTN_API FileDialog
	{
		public:
			static FileDialogResult Open(const std::string& p_FilterList, const std::string& p_DefaultPath, std::string& p_OutPath);
			static FileDialogResult OpenMultiple(const std::string& p_FilterList, const std::string& p_DefaultPath, std::vector<std::string>& p_OutPaths);
			static FileDialogResult Save(const std::string& p_FilterList, const std::string& p_DefaultPath, std::string& p_OutPath);
			static FileDialogResult PickFolder(const std::string& p_DefaultPath, std::string& p_OutPath);
	};

} // namespace KTN