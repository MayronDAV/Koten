#pragma once
#include "EditorPanel.h"
#include "Koten/Core/Engine.h"
#include "Koten/Project/Project.h"


namespace KTN
{
	class ProjectExporterPanel : public EditorPanel
	{
		public:
			ProjectExporterPanel();
			~ProjectExporterPanel() override = default;

			void Open();
			void OnImgui() override;

		private:
			void ExportProject(const std::string& p_Folder);
			void Clean();

		private:
			std::string m_FolderPath = "";

			ProjectConfig m_Config = {};
			Settings m_Settings = {};
			bool m_First = true;
			bool m_ChangedTab = false;
			int m_Tab = 0;

			Ref<Texture2D> m_Icon = nullptr;
	};

} // namespace KTN
