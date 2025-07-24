#include "ContentBrowserPanel.h"
#include "Editor.h"
#include "SettingsPanel.h"
#include "AssetImporterPanel.h"

// lib
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>


namespace KTN
{
	ContentBrowserPanel::ContentBrowserPanel()
		: EditorPanel("Content Browser"), m_BaseDir("Assets")
	{
		KTN_PROFILE_FUNCTION();

		m_CurrentDir = m_BaseDir;

		m_DirectoryIcon = TextureImporter::LoadTexture2D("Resources/Icons/DirectoryIcon.png");
		m_FileIcon = TextureImporter::LoadTexture2D("Resources/Icons/FileIcon.png");
	}

	ContentBrowserPanel::ContentBrowserPanel(const std::string& p_BaseDir)
		: EditorPanel("Content Browser"), m_BaseDir(p_BaseDir)
	{
		KTN_PROFILE_FUNCTION();

		m_CurrentDir = m_BaseDir;

		m_DirectoryIcon = TextureImporter::LoadTexture2D("Resources/Icons/DirectoryIcon.png");
		m_FileIcon = TextureImporter::LoadTexture2D("Resources/Icons/FileIcon.png");
	}

	void ContentBrowserPanel::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		ImGui::Begin(m_Name.c_str(), &m_Active);
		{
			DrawContentTopPanel();

			ImGuiStyle& style = ImGui::GetStyle();
			const float padding = style.WindowPadding.x;
			const ImVec2 availableSize = ImGui::GetContentRegionAvail();
			const float treeWidth = availableSize.x * 0.35f;

			ImGui::BeginChild("##Tree", ImVec2(treeWidth, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
			{
				DrawContentTreeNode(m_BaseDir);
			}
			ImGui::EndChild();

			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - padding);
			ImGui::BeginChild("##Browser", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
			{
				if (m_SearchResults.empty())
					DrawContentBrowser();
				else
					DrawContentSearchResult();
			}
			ImGui::EndChild();

			PopupCreateFileDir();

			for (const auto& path : m_ToDelete)
			{
				if (m_SelectedPath == path.string())
					m_SelectedPath = "";

				if (std::filesystem::exists(path))
					std::filesystem::remove_all(path);
			}
			m_ToDelete.clear();
		}
		ImGui::End();
	}

	Tab ContentBrowserPanel::GetSettingsTab() const
	{
		Tab tab = {};
		tab.Name = m_Name;
		tab.Content = [&]()
		{
			float padding = 120.0f;

			ImGui::Text("Thumbail Size"); ImGui::SameLine(padding);
			ImGui::DragFloat("##ThumbnailSize", (float*)&m_ThumbnailSize, 1.0f, 16.0f, 128.0f, "%.0f px");

			ImGui::Text("Padding"); ImGui::SameLine(padding);
			ImGui::DragFloat("##Padding", (float*)&m_Padding, 1.0f, 0.0f, 64.0f, "%.0f px");
		};

		return tab;
	}

	void ContentBrowserPanel::DrawContentTreeNode(const std::filesystem::path& p_Path, int p_Depth)
	{
		KTN_PROFILE_FUNCTION();

		for (const auto& entry : std::filesystem::directory_iterator(p_Path))
		{
			std::string name = entry.path().filename().string();
			std::string path = entry.path().string();
			bool isDirectory = entry.is_directory();

			ImGui::PushID(name.c_str());

			bool isAncestorOfSelected = false;
			if (!m_SelectedPath.empty() && isDirectory)
			{
				std::string dirPath = path.back() != '/' ? path + '/' : path;			
				isAncestorOfSelected = (m_SelectedPath == path) || (m_SelectedPath.find(dirPath) == 0);
			}

			ImGuiTreeNodeFlags flags = 
				ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_AllowOverlap |
				ImGuiTreeNodeFlags_FramePadding;

			if (m_SelectedPath == path || isAncestorOfSelected)
				flags |= ImGuiTreeNodeFlags_Selected;

			if (!isDirectory)
				flags |= ImGuiTreeNodeFlags_Leaf;

			std::string icon = ICON_MDI_FOLDER;
			if (isDirectory)
			{
				bool isOpen = ImGui::TreeNodeUpdateNextOpen(ImGui::GetID(path.c_str()), 0);
				if (isOpen)
					icon = ICON_MDI_FOLDER_OPEN;
				else
					icon = ICON_MDI_FOLDER;
			}
			else if (entry.path().extension() == ".png" || entry.path().extension() == ".jpg" || entry.path().extension() == "jpeg")
			{
				icon = ICON_MDI_FILE_IMAGE;
			}
			else
			{
				icon = ICON_MDI_FILE;
			}

			if (m_RenamePath == path && m_ContentTreeFocused)
			{
				if (m_SelectedPath == path)
					m_SelectedPath = "";

				static char renameBuffer[120] = { 0 };

				if (ImGui::GetCurrentContext()->ActiveId == 0)
				{
					Strncpy(renameBuffer, name.c_str(), sizeof(renameBuffer));
				}

				ImGuiWindow* window = ImGui::GetCurrentWindow();
				ImGuiContext& g = *GImGui;
				const ImGuiStyle& style = g.Style;

				const bool displayFrame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
				const ImVec2 padding = (displayFrame || (flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

				{
					const float arrowSize = g.FontSize + (padding.x * 2.0f);
					const float textOffsetX = (p_Depth * arrowSize) + (displayFrame ? padding.x * 3 : padding.x * 2) + g.FontSize;

					ImGui::SetCursorPosX(textOffsetX - (entry.is_directory() ? 0.0f : 0.05f));
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding.y);
				}

				ImGui::Text((icon + " ").c_str());

				ImGui::SameLine();

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				
				const ImVec2 framePadding = ImGui::GetStyle().FramePadding;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ( padding.x * 2.0f ));
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - framePadding.y);

				ImGui::SetKeyboardFocusHere();

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, framePadding.y));
				if (ImGui::InputText("##rename", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
				{
					std::filesystem::path newPath = entry.path().parent_path() / renameBuffer;
					if (!std::filesystem::exists(newPath))
						std::filesystem::rename(entry.path(), newPath);

					m_RenamePath.clear();
				}
				ImGui::PopStyleVar();

				if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
					m_RenamePath.clear();
				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
					m_RenamePath.clear();
			}
			else
			{
				bool open = ImGui::TreeNodeEx(path.c_str(), flags, (icon + " " + name).c_str());

				if (!isDirectory && ImGui::BeginDragDropSource())
				{
					auto relativePath = m_BaseDir / std::filesystem::relative(entry.path(), m_BaseDir);
					std::wstring wstr = relativePath.wstring();
					const wchar_t* itemPath = wstr.c_str();
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));

					ImGui::EndDragDropSource();
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					m_ContentTreeFocused = false;
					if (isDirectory)
						m_CurrentDir = entry.path();
					else if (entry.path().extension() == ".ktscn")
					{
						SceneManager::Load(AssetManager::Get()->ImportAsset(AssetType::Scene, entry.path().string()), LoadMode::Single);
					}
				}

				if (ImGui::IsItemClicked())
				{
					m_SelectedPath = path;
					m_ContentTreeFocused = true;
				}

				if (ImGui::BeginPopupContextItem())
				{
					m_ContentTreeFocused = true;
					PopupContextMenuItems(entry.path());
					ImGui::EndPopup();
				}

				if (open)
				{
					if (entry.is_directory())
					{
						int depth = p_Depth + 1;
						DrawContentTreeNode(entry.path(), depth);
					}

					ImGui::TreePop();
				}
			}
			ImGui::PopID();
		}
	}

	void ContentBrowserPanel::PopupContextMenuItems(const std::filesystem::path& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		if (std::filesystem::is_directory(p_Path))
		{
			if (ImGui::MenuItem(ICON_MDI_ARROW_EXPAND " Open"))
			{
				m_CurrentDir = p_Path;
			}
			if (ImGui::MenuItem(ICON_MDI_FOLDER_PLUS " Folder"))
			{
				m_OpenPopupCreateFileDir = true;
				m_IsDirectoryCreateFileDir = true;
				m_CreateFileDirPath = p_Path;
			}
			if (ImGui::MenuItem(ICON_MDI_FILE_PLUS " File"))
			{
				m_OpenPopupCreateFileDir = true;
				m_IsDirectoryCreateFileDir = false;
				m_CreateFileDirPath = p_Path;
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::MenuItem(ICON_MDI_RENAME_BOX " Rename"))
			{
				m_RenamePath = p_Path;
			}
			if (ImGui::MenuItem(ICON_MDI_DELETE_FOREVER " Delete"))
			{
				m_ToDelete.push_back(p_Path);
			}
		}
		else
		{
			if (p_Path.extension() == ".ktscn")
			{
				if (ImGui::MenuItem(ICON_MDI_OPEN_IN_APP " Open"))
				{
					SceneManager::Load(AssetManager::Get()->ImportAsset(AssetType::Scene, p_Path.string()), LoadMode::Single);
				}
			}

			if (ImGui::MenuItem(ICON_MDI_RENAME_BOX " Rename"))
			{
				m_RenamePath = p_Path;
			}

			if (ImGui::MenuItem(ICON_MDI_DELETE_FOREVER " Delete"))
			{
				m_ToDelete.push_back(p_Path);
			}
		}
	}

	void ContentBrowserPanel::PopupCreateFileDir()
	{
		if (!m_OpenPopupCreateFileDir)
			return;

		ImGui::OpenPopup("Create File/Folder");
		if (ImGui::BeginPopupModal("Create File/Folder", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char name[256] = "";
			name[0] = '\0';

			ImGui::Text("Enter the name of the new %s:", m_IsDirectoryCreateFileDir ? "folder" : "file");

			ImGui::Separator();

			ImGui::Text((m_CreateFileDirPath.filename().string() + " / ").c_str());

			ImGui::SameLine();

			ImGui::SetKeyboardFocusHere();
			if (ImGui::InputText("##Name", name, sizeof(name), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::filesystem::path newPath = m_CreateFileDirPath / name;
				if (m_IsDirectoryCreateFileDir)
					std::filesystem::create_directory(newPath);
				else
					FileSystem::WriteTextFile(newPath.string(), "");

				m_OpenPopupCreateFileDir = false;
			}

			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				std::filesystem::path newPath = m_CreateFileDirPath / name;
				if (m_IsDirectoryCreateFileDir)
					std::filesystem::create_directory(newPath);
				else
					FileSystem::WriteTextFile(newPath.string(), "");

				m_OpenPopupCreateFileDir = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
				m_OpenPopupCreateFileDir = false;

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				m_OpenPopupCreateFileDir = false;

			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::DrawContentBrowser()
	{
		KTN_PROFILE_FUNCTION();

		float cellSize = m_ThumbnailSize + m_Padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		columnCount = std::max(1, columnCount);

		std::vector<std::filesystem::directory_entry> entries;
		for (auto& entry : std::filesystem::directory_iterator(m_CurrentDir))
			entries.push_back(entry);

		if (ImGui::BeginTable("##ContentBrowser", columnCount))
		{
			ImGuiListClipper clipper;
			clipper.Begin((int)std::ceil((float)entries.size() / columnCount), m_ThumbnailSize + ImGui::GetTextLineHeight() * 2);

			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
				{
					ImGui::TableNextRow();

					for (int column = 0; column < columnCount; column++)
					{
						int index = row * columnCount + column;
						if (index >= entries.size())
							continue;

						ImGui::TableSetColumnIndex(column);
						const auto& entry = entries[index];
						std::string filename = entry.path().filename().string();
						bool isDirectory = entry.is_directory();

						// Draw item
						ImGui::PushID(filename.c_str());

						// Thumbnail button
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						UI::ImageButton(isDirectory ? m_DirectoryIcon : m_FileIcon, { m_ThumbnailSize, m_ThumbnailSize });

						if (!std::filesystem::is_directory(entry.path()) && ImGui::BeginDragDropSource())
						{
							std::filesystem::path relativePath = m_BaseDir / std::filesystem::relative(entry.path(), m_BaseDir);
							std::wstring wstr = relativePath.wstring();
							const wchar_t* itemPath = wstr.c_str();
							ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));

							ImGui::EndDragDropSource();
						}

						ImGui::PopStyleColor();

						// Handle interactions
						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
						{
							m_ContentTreeFocused = false;
							if (isDirectory)
								m_CurrentDir /= entry.path().filename();
							else if (entry.path().extension() == ".ktscn")
								SceneManager::Load(AssetManager::Get()->ImportAsset(AssetType::Scene, entry.path().string()), LoadMode::Single);
						}

						if (ImGui::IsItemClicked())
						{
							m_SelectedPath = entry.path().string();
							m_ContentTreeFocused = false;
						}

						// Context menu
						if (ImGui::BeginPopupContextItem())
						{
							m_ContentTreeFocused = false;
							PopupContextMenuItems(entry.path());
							ImGui::EndPopup();
						}

						// Filename text or rename input
						if (m_RenamePath == entry.path() && !m_ContentTreeFocused)
						{
							static char renameBuffer[256];
							if (ImGui::GetCurrentContext()->ActiveId == 0)
							{
								Strncpy(renameBuffer, filename.c_str(), sizeof(renameBuffer));
							}

							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
							ImGui::SetNextItemWidth(m_ThumbnailSize);
							ImGui::SetKeyboardFocusHere();
							if (ImGui::InputText("##rename", renameBuffer, sizeof(renameBuffer),
								ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
							{
								auto newPath = entry.path().parent_path() / renameBuffer;
								if (!exists(newPath))
									rename(entry.path(), newPath);
								m_RenamePath.clear();
							}
							ImGui::PopStyleVar();

							if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
								m_RenamePath.clear();
							if (ImGui::IsKeyPressed(ImGuiKey_Escape))
								m_RenamePath.clear();
						}
						else
						{
							ImGui::TextWrapped("%s", filename.c_str());
						}

						ImGui::PopID();
					}
				}
			}
			ImGui::EndTable();
		}
	}

	void ContentBrowserPanel::DrawContentTopPanel()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		static char searchText[256] = { 0 };

		ImGui::BeginChild("TopPanel", ImVec2(0, 30), false);
		{
			ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
			if (ImGui::Button(ICON_MDI_ARROW_LEFT, { 0, 0 }))
			{
				if (m_CurrentDir != std::filesystem::path(m_BaseDir))
				{
					m_FuturePaths.push(m_CurrentDir);
					m_CurrentDir = m_CurrentDir.parent_path();
				}
			}

			ImGui::SameLine();

			if (ImGui::Button(ICON_MDI_ARROW_RIGHT, { 0, 0 }))
			{
				if (!m_FuturePaths.empty())
				{
					m_CurrentDir = m_FuturePaths.top();
					m_FuturePaths.pop();
				}
			}

			ImGui::PopStyleColor();

			ImGui::SameLine();

			// Calculate width for the cancel button
			float cancelButtonWidth = (strlen(searchText) > 0) ? ImGui::GetFrameHeight() : 0.0f;

			// Adjust input text width to make space for the button
			float inputWidth = ImGui::GetContentRegionAvail().x - cancelButtonWidth - 300.0f;
			ImGui::SetNextItemWidth(inputWidth);

			if (ImGui::InputTextWithHint("##Search", "Search...", searchText, sizeof(searchText), ImGuiInputTextFlags_None))
			{
				m_SearchResults.clear();
				if (strlen(searchText) > 0)
				{
					FileSystem::Search(searchText, m_BaseDir.string(), m_SearchResults);
				}
			}

			// Show cancel button only when there's text in the search field
			if (strlen(searchText) > 0)
			{
				ImGui::SameLine();
				if (ImGui::Button(ICON_MDI_CANCEL, ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
				{
					searchText[0] = '\0';
					m_SearchResults.clear();
				}
			}

			ImGui::SameLine();

			std::vector<std::filesystem::path> pathSegments;
			// Always start with the base directory
			pathSegments.push_back(m_BaseDir.filename());

			// Add remaining segments if we're not at the base directory
			if (m_CurrentDir != m_BaseDir)
			{
				auto currentRelPath = std::filesystem::relative(m_CurrentDir, m_BaseDir);
				std::filesystem::path pathAccumulator = m_BaseDir.filename();

				for (const auto& part : currentRelPath)
				{
					pathAccumulator /= part;
					pathSegments.push_back(pathAccumulator);
				}
			}

			bool first = true;
			for (const auto& segment : pathSegments)
			{
				std::string filenameStr = segment.filename().string();

				if (!first)
				{
					ImGui::SameLine();
					ImGui::Text("/"); // Separator
					ImGui::SameLine();
				}

				ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.3f, 0.35f, 0.5f });
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.3f, 0.3f, 0.35f, 0.5f });

				if (ImGui::Button(filenameStr.c_str()))
				{
					if (segment == m_BaseDir.filename())
					{
						// Clicked on base directory
						if (m_CurrentDir != m_BaseDir)
						{
							ClearStack();
							m_CurrentDir = m_BaseDir;
						}
					}
					else
					{
						// Clicked on a subdirectory
						auto fullPath = m_BaseDir / segment.relative_path();
						if (m_CurrentDir != fullPath)
						{
							ClearStack();
							m_CurrentDir = fullPath;
						}
					}
				}
				ImGui::PopStyleColor(3);

				first = false;
			}
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}

	void ContentBrowserPanel::DrawContentSearchResult()
	{
		if (m_SearchResults.empty()) return;

		float cellSize = m_ThumbnailSize + m_Padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		columnCount = std::max(1, columnCount);


		if (ImGui::BeginTable("##ContentBrowser", columnCount))
		{
			ImGuiListClipper clipper;
			clipper.Begin((int)std::ceil((float)m_SearchResults.size() / columnCount), m_ThumbnailSize + ImGui::GetTextLineHeight() * 2);

			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
				{
					ImGui::TableNextRow();

					for (int column = 0; column < columnCount; column++)
					{
						int index = row * columnCount + column;
						if (index >= (int)m_SearchResults.size())
							continue;

						ImGui::TableSetColumnIndex(column);
						const auto& entry = m_SearchResults.at(index);
						std::string filename = entry.filename().string();
						bool isDirectory = std::filesystem::is_directory(entry);

						// Draw item
						ImGui::PushID(filename.c_str());

						// Thumbnail button
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						UI::ImageButton(isDirectory ? m_DirectoryIcon : m_FileIcon, { m_ThumbnailSize, m_ThumbnailSize });

						if (!isDirectory && ImGui::BeginDragDropSource())
						{
							std::filesystem::path relativePath = "Assets" / std::filesystem::relative(entry, m_BaseDir);
							std::wstring wstr = relativePath.wstring();
							const wchar_t* itemPath = wstr.c_str();
							ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));

							ImGui::EndDragDropSource();
						}

						ImGui::PopStyleColor();

						// Handle interactions
						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
						{
							m_ContentTreeFocused = false;
							if (isDirectory)
								m_CurrentDir /= entry.filename();
							else if (entry.extension() == ".ktscn")
								SceneManager::Load(AssetManager::Get()->ImportAsset(AssetType::Scene, entry.string()), LoadMode::Single);
						}

						if (ImGui::IsItemClicked())
						{
							m_SelectedPath = entry.string();
							m_ContentTreeFocused = false;
						}

						// Context menu
						if (ImGui::BeginPopupContextItem())
						{
							m_ContentTreeFocused = false;
							PopupContextMenuItems(entry);
							ImGui::EndPopup();
						}

						// Filename text or rename input
						if (m_RenamePath == entry && !m_ContentTreeFocused)
						{
							static char renameBuffer[256];
							if (ImGui::GetCurrentContext()->ActiveId == 0)
							{
								Strncpy(renameBuffer, filename.c_str(), sizeof(renameBuffer));
							}

							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
							ImGui::SetNextItemWidth(m_ThumbnailSize);
							ImGui::SetKeyboardFocusHere();
							if (ImGui::InputText("##rename", renameBuffer, sizeof(renameBuffer),
								ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
							{
								auto newPath = entry.parent_path() / renameBuffer;
								if (!exists(newPath))
									rename(entry, newPath);
								m_RenamePath.clear();
							}
							ImGui::PopStyleVar();

							if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
								m_RenamePath.clear();
							if (ImGui::IsKeyPressed(ImGuiKey_Escape))
								m_RenamePath.clear();
						}
						else
						{
							ImGui::TextWrapped("%s", filename.c_str());
						}

						ImGui::PopID();
					}
				}
			}
			ImGui::EndTable();
		}

	}

} // namespace KTN
