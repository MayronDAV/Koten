#include "AssetRegistryPanel.h"
#include "Editor.h"
#include "AssetImporterPanel.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>



namespace KTN
{
	AssetRegistryPanel::AssetRegistryPanel()
		: EditorPanel("AssetRegistry")
	{
	}

	void AssetRegistryPanel::OnImgui()
	{
		ImGui::Begin(m_Name.c_str(), &m_Active, ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu(ICON_MDI_HELP_CIRCLE_OUTLINE " Ajuda"))
			{
				ImGui::MenuItem("Right click on any item or space for options");

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (ImGui::BeginTable("##AssetRegistryTable", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 170.0f);
			ImGui::TableSetupColumn("Filename");
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableHeadersRow();

			std::vector<AssetHandle> assetsRemoved;

			const auto& assetRegistry = AssetManager::Get()->GetAssetRegistry();
			auto it = assetRegistry.begin();

			ImGuiListClipper clipper;
			clipper.Begin(assetRegistry.size());
			static AssetHandle currentID;

			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
				{
					if (it == assetRegistry.end())
						break;

					std::pair<AssetHandle, AssetMetadata> asset = *it;

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					std::string id = std::to_string(asset.first);
					ImGui::Text(id.c_str());

					bool hovered_item = ImGui::IsItemHovered();

					ImGui::TableSetColumnIndex(1);

					ImGui::Text(asset.second.FilePath.c_str());

					hovered_item = hovered_item || ImGui::IsItemHovered();

					if (ImGui::IsItemHovered())
						currentID = asset.first;

					ImGui::PushID(currentID + asset.first);
					if (ImGui::BeginPopupContextWindow(0, 1))
					{
						if (ImGui::MenuItem("Import"))
						{
							m_Editor->GetAssetImporterPanel()->Open();
						}

						if (ImGui::MenuItem("Delete"))
							assetsRemoved.push_back(currentID);

						ImGui::EndPopup();
					}
					ImGui::PopID();

					if (hovered_item)
					{
						ImGui::BeginTooltip();

						ImGui::Text("Type: %s", GetAssetTypeName(asset.second.Type));

						std::string path = std::filesystem::relative(asset.second.FilePath, Project::GetAssetDirectory()).string();
						ImGui::Text("Filepath: %s", path.c_str());

						ImGui::EndTooltip();
					}

					it++;
				}

				if (!assetsRemoved.empty())
				{
					for (const auto& id : assetsRemoved)
					{
						if (AssetManager::Get()->IsAssetHandleValid(id))
							AssetManager::Get()->RemoveAsset(id);
					}

					assetsRemoved.clear();
				}
			}

			ImGui::EndTable();
		}

		ImGui::End();
	}


} // namespace KTN
