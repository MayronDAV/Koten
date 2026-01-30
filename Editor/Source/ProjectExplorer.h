#pragma once
#include "Editor.h"


namespace KTN
{
	class ProjectExplorer : public Layer
	{
		public:
			ProjectExplorer();
			~ProjectExplorer();

			void OnImgui() override;

			enum class ProjectExplorerState
			{
				MainMenu,
				CreatingProject
			};

		private:
			void ShowMainMenu(const glm::vec2& p_Size);
			void ShowCreateProjectForm(const glm::vec2& p_Size);
			void CreateProject();

		private:
			std::string m_ProjectPath;
			bool m_ShouldOpenProject = false;

			ProjectExplorerState m_State = ProjectExplorerState::MainMenu;
			std::string m_NewProjectFolder = "";
			std::string m_NewProjectName = "";
	};

} // namespace KTN