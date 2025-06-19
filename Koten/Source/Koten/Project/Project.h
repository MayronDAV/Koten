#pragma once

#include "Koten/Core/Base.h"

// std
#include <string>
#include <filesystem>



namespace KTN 
{
	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory;
	};

	class KTN_API Project
	{
		public:
			static const std::filesystem::path& GetProjectDirectory();
			static std::filesystem::path GetAssetDirectory();
			static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& p_Path);

			static Ref<Project> New();
			static Ref<Project> Load(const std::filesystem::path& p_Path);
			static bool SaveActive(const std::filesystem::path& p_Path);

			static Ref<Project> GetActive() { return s_ActiveProject; }

		public:
			ProjectConfig& GetConfig() { return m_Config; }

		private:
			ProjectConfig m_Config;
			std::filesystem::path m_ProjectDirectory;

			inline static Ref<Project> s_ActiveProject;
	};

} // namespace KTN