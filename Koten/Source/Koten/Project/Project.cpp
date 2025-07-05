#include "ktnpch.h"
#include "Project.h"
#include "ProjectSerializer.h"



namespace KTN
{
	const std::filesystem::path& Project::GetProjectDirectory()
	{
		KTN_CORE_ASSERT(s_ActiveProject);
		return s_ActiveProject->m_ProjectDirectory;
	}

	std::filesystem::path Project::GetAssetDirectory()
	{
		KTN_CORE_ASSERT(s_ActiveProject);
		return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
	}

	std::filesystem::path Project::GetAssetFileSystemPath(const std::filesystem::path& p_Path)
	{
		KTN_CORE_ASSERT(s_ActiveProject);
		return GetAssetDirectory() / p_Path;
	}

	Ref<Project> Project::New()
	{
		s_ActiveProject = CreateUnique<Project>();
		return s_ActiveProject;
	}

	Ref<Project> Project::New(const std::filesystem::path& p_FolderPath)
	{
		ProjectConfig config = {};
		config.Name = p_FolderPath.stem().string();
		config.AssetDirectory = "Assets";
		config.StartScene = 0;

		return New(p_FolderPath, config);
	}

	Ref<Project> Project::New(const std::filesystem::path& p_FolderPath, const ProjectConfig& p_Config)
	{
		auto project = New();
		project->m_Config = p_Config;
		SaveActive(p_FolderPath / (p_Config.Name + ".ktproj"));

		FileSystem::CreateDirectories((p_FolderPath / p_Config.AssetDirectory / "Scenes").string());
		FileSystem::CreateDirectories((p_FolderPath / p_Config.AssetDirectory / "Scripts").string());
		FileSystem::CreateDirectories((p_FolderPath / p_Config.AssetDirectory / "Textures").string());

		return project;
	}

	Ref<Project> Project::Load(const std::filesystem::path& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		Ref<Project> project = CreateRef<Project>();

		ProjectSerializer serializer(project);
		if (serializer.Deserialize(p_Path))
		{
			project->m_ProjectDirectory = p_Path.parent_path();
			s_ActiveProject = project;
			s_ActiveProject->m_AssetManager->DeserializeAssetRegistry();
			return s_ActiveProject;
		}

		return nullptr;
	}

	bool Project::SaveActive(const std::filesystem::path& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(p_Path))
		{
			s_ActiveProject->m_ProjectDirectory = p_Path.parent_path();
			return true;
		}

		return false;
	}

	Project::Project()
		: m_Config({}), m_AssetManager(AssetManager::Create())
	{
	}

} // namespace KTN