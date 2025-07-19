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
		AssetHandle StartScene = 0;
		std::filesystem::path AssetDirectory = "Assets";

		std::string IconPath = "";
	};

	class KTN_API Project
	{
		public:
			static const std::filesystem::path& GetProjectDirectory();
			static std::filesystem::path GetAssetDirectory();
			static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& p_Path);

			static Ref<Project> New();
			static Ref<Project> New(const std::filesystem::path& p_FolderPath);
			static Ref<Project> New(const std::filesystem::path& p_FolderPath, const ProjectConfig& p_Config);
			static Ref<Project> Load(const std::filesystem::path& p_Path);
			static bool SaveActive(const std::filesystem::path& p_Path);

			static Ref<Project> LoadRuntime(const std::filesystem::path& p_Path);
			static bool SaveActiveRuntime(const std::filesystem::path& p_Path);

			static Ref<Project> GetActive() { return s_ActiveProject; }

		public:
			Project();
			Project(const ProjectConfig& p_Config) : m_Config(p_Config) {}

			Ref<AssetManager> GetAssetManager() { return m_AssetManager; }
			ProjectConfig& GetConfig() { return m_Config; }
			bool IsRuntime() const { return m_IsRuntime; }

		private:
			ProjectConfig m_Config;
			std::filesystem::path m_ProjectDirectory;
			Ref<AssetManager> m_AssetManager;
			bool m_IsRuntime = false;

			inline static Ref<Project> s_ActiveProject = nullptr;
	};

} // namespace KTN