#pragma once

#include "Koten/Core/Base.h"
#include "Koten/Asset/AssetManager.h"

// std
#include <string>
#include <filesystem>



namespace KTN 
{
	struct ProjectConfig
	{
		std::string Name = "Untitled";

		AssetHandle StartScene;

		std::filesystem::path AssetDirectory;
	};

	class KTN_API Project
	{
		public:
			static const std::filesystem::path& GetProjectDirectory();
			static std::filesystem::path GetAssetDirectory();
			static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& p_Path);

			static Ref<Project> New();
			static Ref<Project> New(const std::filesystem::path& p_FolderPath);
			static Ref<Project> Load(const std::filesystem::path& p_Path);
			static bool SaveActive(const std::filesystem::path& p_Path);

			static Ref<Project> GetActive() { return s_ActiveProject; }

		public:
			Project();

			Ref<AssetManager> GetAssetManager() { return m_AssetManager; }
			ProjectConfig& GetConfig() { return m_Config; }

		private:
			ProjectConfig m_Config;
			std::filesystem::path m_ProjectDirectory;
			Ref<AssetManager> m_AssetManager;

			inline static Ref<Project> s_ActiveProject;
	};

} // namespace KTN