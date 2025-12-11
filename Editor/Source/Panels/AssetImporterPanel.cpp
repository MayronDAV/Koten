#include "AssetImporterPanel.h"
#include "Editor.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>



namespace KTN
{
	static std::vector<void*> s_MemTrash;

	AssetImporterPanel::~AssetImporterPanel()
	{
		for (auto& ptr : s_MemTrash)
		{
			if (ptr != nullptr)
			{
				delete ptr;
				ptr = nullptr;
			}
		}
		s_MemTrash.clear();
	}

	void AssetImporterPanel::OnImgui()
	{
		KTN_PROFILE_FUNCTION();
		if (!m_Active)
			return;

		ImGui::Begin("AssetImporter", nullptr);
		{
			static bool typeChanged = false;

			UI::InputText("File Path", m_Path, true, ImGuiInputTextFlags_AutoSelectAll, 2.0f, true);

			{
				int currentItem = (int)m_Type;
				static const char* items[] = { "None", "Scene", "Font", "Texture2D", "PhysicsMaterial2D"};
				static const int itemsCount = IM_ARRAYSIZE(items);

				if (UI::Combo("Asset Type", items[currentItem], items, itemsCount, &currentItem))
				{
					m_Type = (AssetType)currentItem;
					typeChanged = true;
				}
			}

			static bool forceImport = false;
			ImGui::Checkbox("Force Import", &forceImport);

			static bool useAssetData = true;
			ImGui::Checkbox("Use AssetData", &useAssetData);

			if (typeChanged || m_First)
			{
				if (useAssetData)
				{
					if (m_Type == AssetType::Font)
						m_Metadata.AssetData = new DFFontConfig();
				
					if (m_Type == AssetType::Texture2D)
						m_Metadata.AssetData = new TextureSpecification();

					if (m_Metadata.AssetData)
						s_MemTrash.push_back(m_Metadata.AssetData);
				}

				forceImport = false;
				typeChanged = false;
				m_First = false;
			}

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			if (useAssetData)
			{
				auto size = ImGui::GetContentRegionAvail();
				ImGui::BeginChild("Asset Metadata", ImVec2(size.x, size.y - (lineHeight * 1.5f)), true);
				{
					if (m_Type == AssetType::Font)
					{
						DFFontConfig* config = static_cast<DFFontConfig*>(m_Metadata.AssetData);

						// ImageType
						{
							int currentItem = (int)config->ImageType;
							static const char* items[] = { "HARD_MASK", "SOFT_MASK", "SDF", "PSDF", "MSDF", "MTSDF" };
							static const int itemsCount = IM_ARRAYSIZE(items);

							if (UI::Combo("Atlas ImageType", items[currentItem], items, itemsCount, &currentItem))
							{
								config->ImageType = (FontImageType)currentItem;
							}
						}

						// GlyphIdentifier
						{
							int currentItem = (int)config->GlyphIdentifier;
							static const char* items[] = { "GLYPH_INDEX", "UNICODE_CODEPOINT" };
							static const int itemsCount = IM_ARRAYSIZE(items);
							if (UI::Combo("Glyph Identifier", items[currentItem], items, itemsCount, &currentItem))
							{
								config->GlyphIdentifier = (GlyphIdentifierType)currentItem;
							}
						}

						// ImageFormat
						{
							int currentItem = (int)config->ImageFormat;
							static const char* items[] = { "UNSPECIFIED", "PNG", "BMP", "TIFF", "TEXT", "TEXT_FLOAT", "BINARY", "BINARY_FLOAT", "BINARY_FLOAT_BE" };
							static const int itemsCount = IM_ARRAYSIZE(items);
							if (UI::Combo("Atlas Image Format", items[currentItem], items, itemsCount, &currentItem))
							{
								config->ImageFormat = (FontImageFormat)currentItem;
							}
						}

						ImGui::InputFloat("Em Size", &config->EmSize, 1.0f, 10.0f, "%.2f");
						ImGui::InputDouble("Px Range", &config->PxRange, 0.1, 1.0, "%.2f");
						ImGui::InputDouble("Miter Limit", &config->MiterLimit, 0.1, 1.0, "%.2f");
						ImGui::InputDouble("Angle Threshold", &config->AngleThreshold, 0.1, 1.0, "%.2f");
						ImGui::InputDouble("Font Scale", &config->FontScale, 0.1, 1.0, "%.2f");
						ImGui::InputInt("Thread Count", &config->ThreadCount);
						ImGui::Checkbox("Expensive Coloring", &config->ExpensiveColoring);
						ImGui::Checkbox("Fixed Scale", &config->FixedScale);
						ImGui::Checkbox("Overlap Support", &config->OverlapSupport);
						ImGui::Checkbox("Scanline Pass", &config->ScanlinePass);
						ImGui::Checkbox("Use Default Charset", &config->UseDefaultCharset);
						if (!config->UseDefaultCharset)
						{
							ImGui::Text("Charset Ranges:");
							ImGui::SameLine();
							if (ImGui::SmallButton(ICON_MDI_PLUS))
							{
								ImGui::OpenPopup("Add Range");
							}

							ImGui::BeginChild("CharsetRanges", ImVec2(200, 0), true);
							{
								ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));

								float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
								for (size_t i = 0; i < config->CharsetRanges.size(); )
								{
									ImGui::PushID(static_cast<int>(i));

									const float inputWidth = ImGui::CalcTextSize("0x0000").x + 8.0f;

									uint32_t rangeStart = config->CharsetRanges[i].first;
									char startBuf[7];
									snprintf(startBuf, sizeof(startBuf), "0x%04X", rangeStart);

									ImGui::SetNextItemWidth(inputWidth);
									if (ImGui::InputText("##Start", startBuf, sizeof(startBuf),
										ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
									{
										uint32_t newVal;
										if (SScanf(startBuf, "%X", &newVal) == 1)
										{
											config->CharsetRanges[i].first = newVal;
										}
									}

									ImGui::SameLine();
									ImGui::Text("to");
									ImGui::SameLine();

									uint32_t rangeEnd = config->CharsetRanges[i].second;
									char endBuf[7];
									snprintf(endBuf, sizeof(endBuf), "0x%04X", rangeEnd);

									ImGui::SetNextItemWidth(inputWidth);
									if (ImGui::InputText("##End", endBuf, sizeof(endBuf),
										ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
									{
										uint32_t newVal;
										if (SScanf(endBuf, "%X", &newVal) == 1)
										{
											config->CharsetRanges[i].second = newVal;
										}
									}

									ImGui::SameLine();
									if (ImGui::Button("X", { lineHeight, lineHeight }))
									{
										config->CharsetRanges.erase(config->CharsetRanges.begin() + i);
										ImGui::PopID();
										continue;
									}

									ImGui::PopID();
									i++;
								}

								ImGui::PopStyleVar(2);
							}
							ImGui::EndChild();

							if (ImGui::BeginPopup("Add Range"))
							{
								static char newStartBuf[7] = "0x0020";
								static char newEndBuf[7] = "0x007F";

								const float inputWidth = ImGui::CalcTextSize("0x0000").x + 8.0f;

								ImGui::SetNextItemWidth(inputWidth);
								ImGui::InputText("##Start##New", newStartBuf, sizeof(newStartBuf),
									ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);

								ImGui::SameLine();
								ImGui::Text("to");
								ImGui::SameLine();

								ImGui::SetNextItemWidth(inputWidth);
								ImGui::InputText("##End##New", newEndBuf, sizeof(newEndBuf),
									ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);

								if (ImGui::Button("Add"))
								{
									uint32_t start, end;
									if (SScanf(newStartBuf, "%X", &start) == 1 &&
										SScanf(newEndBuf, "%X", &end) == 1)
									{
										if (start > end) std::swap(start, end);
										config->CharsetRanges.push_back({ start, end });
									}
									Strcpy(newStartBuf, "0x0020");
									Strcpy(newEndBuf, "0x007F");
									ImGui::CloseCurrentPopup();
								}

								ImGui::SameLine();
								if (ImGui::Button("Cancel"))
								{
									ImGui::CloseCurrentPopup();
								}

								ImGui::EndPopup();
							}
						}

					}

					if (m_Type == AssetType::Texture2D)
					{
						TextureSpecification* spec = static_cast<TextureSpecification*>(m_Metadata.AssetData);

						// Filter
						{
							int currentItem = (int)spec->MinFilter - 1;
							static const char* items[] = { "LINEAR", "NEAREST" };
							static const int itemsCount = IM_ARRAYSIZE(items);

							if (UI::Combo("Min Filter", items[currentItem], items, itemsCount, &currentItem))
							{
								spec->MinFilter = (TextureFilter)(currentItem + 1);
							}

							currentItem = (int)spec->MagFilter - 1;
							if (UI::Combo("Mag Filter", items[currentItem], items, itemsCount, &currentItem))
							{
								spec->MagFilter = (TextureFilter)(currentItem + 1);
							}
						}

						// Wrap
						{
							int currentItem = (int)spec->WrapU - 1;
							static const char* items[] = { "REPEAT", "MIRRORED_REPEAT", "CLAMP_TO_EDGE", "CLAMP_TO_BORDER" };
							static const int itemsCount = IM_ARRAYSIZE(items);

							if (UI::Combo("Wrap U", items[currentItem], items, itemsCount, &currentItem))
							{
								spec->WrapU = (TextureWrap)(currentItem + 1);
							}

							currentItem = (int)spec->WrapV - 1;
							if (UI::Combo("Wrap V", items[currentItem], items, itemsCount, &currentItem))
							{
								spec->WrapV = (TextureWrap)(currentItem + 1);
							}
						}

						ImGui::Checkbox("SRGB", &spec->SRGB);
						ImGui::Checkbox("Anisotropy Enable", &spec->AnisotropyEnable);
						ImGui::Checkbox("Generate Mipmaps", &spec->GenerateMips);
						ImGui::ColorEdit4("Border Color", &spec->BorderColor[0]);

						UI::InputText("Debug Name", spec->DebugName, true, ImGuiInputTextFlags_AutoSelectAll, 2.0f, true);
					}

					if (m_Type == AssetType::Scene)
					{
						// TODO
					}
				}
				ImGui::EndChild();
			}

			if (ImGui::Button("Import", { 100.0f, lineHeight }))
			{
				if (m_Type == AssetType::None)
				{
					KTN_CORE_ERROR("Please select a valid asset type to import.");
					m_Metadata = {};
					m_Active = false;
					ImGui::End();
					return;
				}

				auto assetManager = Project::GetActive()->GetAssetManager();
				m_Metadata.FilePath = m_Path;
				m_Metadata.Type = m_Type;
				auto asset = assetManager->ImportAsset(m_Metadata, forceImport);
				if (assetManager->IsAssetHandleValid(asset))
				{
					KTN_CORE_INFO("Successfully imported asset: {}", m_Path);
					if (useAssetData)
						s_MemTrash.pop_back();
					m_Imported = true;
				}
				else
					KTN_CORE_ERROR("Failed to import asset: {}", m_Path);

				m_Metadata = {};
				m_Active = false; // Close the panel after import
				forceImport = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", { 100.0f, lineHeight }))
			{
				m_Metadata = {};
				m_Active = false;
				forceImport = false;
			}
		}
		ImGui::End();
	}
}
