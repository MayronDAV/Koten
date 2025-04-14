#pragma once
#include "Koten/Koten.h"
#include "EditorPanel.h"

// std
#include <vector>
#include <stack>
#include <filesystem>


namespace KTN
{
	struct Tab;

	class ContentBrowserPanel : public EditorPanel
	{
		public:
			ContentBrowserPanel();
			ContentBrowserPanel(const std::string& p_BaseDir);
			~ContentBrowserPanel() override = default;

			void OnImgui() override;

			Tab GetSettingsTab() const;

		private:
			void DrawContentTreeNode(const std::filesystem::path& p_Path, int p_Depth = 0);
			void PopupContextMenuItems(const std::filesystem::path& p_Path);
			void PopupCreateFileDir();

			void DrawContentBrowser();
			void DrawContentTopPanel();
			void DrawContentSearchResult();

			void ClearStack()
			{
				while (!m_FuturePaths.empty())
					m_FuturePaths.pop();
			}


		private:
			std::filesystem::path m_BaseDir;
			std::filesystem::path m_CurrentDir;

			std::string m_SelectedPath;
			std::vector<std::filesystem::path> m_ToDelete;

			std::filesystem::path m_RenamePath;

			std::filesystem::path m_CreateFileDirPath;
			bool m_OpenPopupCreateFileDir = false;
			bool m_IsDirectoryCreateFileDir = false;

			bool m_ContentTreeFocused = false;

			float m_Padding = 16.0f;
			float m_ThumbnailSize = 64.0f;

			Ref<Texture2D> m_FileIcon = nullptr;
			Ref<Texture2D> m_DirectoryIcon = nullptr;

			std::stack<std::filesystem::path> m_FuturePaths;
			std::vector<std::filesystem::path> m_SearchResults;
	};


} // namespace KTN