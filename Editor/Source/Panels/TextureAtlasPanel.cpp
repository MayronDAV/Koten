#include "TextureAtlasPanel.h"
#include "Editor.h"
#include "Koten/Asset/AssetManager.h"
#include "Koten/Asset/TextureAtlasImporter.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>



namespace KTN
{
    namespace
    {
        static void GenerateUVs(Ref<TextureAtlas>& p_Atlas, const glm::vec2& p_GridSize)
        {
            if (!p_Atlas->Texture)
                return;

            auto texture        = AssetManager::Get()->GetAsset<Texture2D>(p_Atlas->Texture);
            if (!texture)
                return;

            glm::vec2 atlasSize = { (float)texture->GetWidth(), (float)texture->GetHeight() };

            int cols            = atlasSize.x / p_GridSize.x;
            int rows            = atlasSize.y / p_GridSize.y;

            int total           = cols * rows;
            p_Atlas->Regions.clear();
            p_Atlas->Regions.reserve(total);

            for (int i = 0; i < total; i++)
            {
                AtlasRegion region = {};

                int x              = i % cols;
                int y              = i / cols;

                float u0           = (x * p_GridSize.x) / (float)atlasSize.x;
                float v0           = 1.0f - ((y * p_GridSize.y) / (float)atlasSize.y);

                float u1           = ((x + 1) * p_GridSize.x) / (float)atlasSize.x;
                float v1           = 1.0f - (((y + 1) * p_GridSize.y) / (float)atlasSize.y);

                region.Position    = { x * p_GridSize.x, y * p_GridSize.y };
                region.UV          = { u0, v0, u1, v1 };
                region.Name        = "Region " + std::to_string(i);
                region.GridSize    = p_GridSize;

                p_Atlas->Regions.push_back(region);
            }

            p_Atlas->BuildRegionLookup();
        }
    } // namespace

    void TextureAtlasPanel::OnImgui()
    {
        KTN_PROFILE_FUNCTION();

        bool wasActive = m_Active;

        if (!m_Atlas)
            return;

        ImGui::Begin(m_Name.c_str(), &m_Active, ImGuiWindowFlags_NoDocking);

        ImGui::BeginTable("AtlasLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner);

        ImGui::TableNextColumn();
        DrawLeftPanel();

        ImGui::TableNextColumn();
        DrawImagePreview();

        ImGui::EndTable();

        ImGui::End();

        if (wasActive && !m_Active)
        {
            m_Atlas = nullptr;
        }
    }

    void TextureAtlasPanel::Open()
    {
        KTN_PROFILE_FUNCTION();

        m_Atlas = CreateRef<TextureAtlas>();
        m_Active = true;
    }

    void TextureAtlasPanel::DrawLeftPanel()
    {
        KTN_PROFILE_FUNCTION();

        auto size = ImGui::GetContentRegionAvail();
        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        ImGui::BeginChild("LeftPanel", { size.x, size.y - (lineHeight * 1.5f) }, false);
        ImGui::PopStyleVar();

        if (ImGui::CollapsingHeader("Settings"))
        {
            ImGui::PushID("LeftPanelSettings");

            if (UI::InputFloat2("Grid Size", m_GridTemp, 64.0f))
            {
            }

            if (ImGui::Button("Generate UVs"))
            {
                if (m_Atlas->Texture)
                    GenerateUVs(m_Atlas, m_GridTemp);

                m_Grid = m_GridTemp;
            }

            if (m_SelectedRegionID)
            {
                ImGui::PushID("RegionSettings");
                auto region = m_Atlas->GetRegion(m_SelectedRegionID);
                if (region)
                {
                    UI::InputText("Name", region->Name, true, 0, 2.0f, true);
                    UI::InputFloat2("Position", region->Position, 0.0f);
                    UI::InputFloat2("Grid Size", region->GridSize, 0.0f);
                }
                ImGui::PopID();
            }

            ImGui::PopID();
        }

        if (ImGui::CollapsingHeader("Regions"))
        {
            if (m_Atlas->Texture)
            {
                float panelWidth = ImGui::GetContentRegionAvail().x;
                float itemWidth = 80.0f;
                int columns = std::max(1, (int)(panelWidth / itemWidth));

                ImGui::Columns(columns, "RegionGrid", false);

                for (const auto& region : m_Atlas->Regions)
                {
                    ImGui::BeginGroup();

                    auto texture = AssetManager::Get()->GetAsset<Texture2D>(m_Atlas->Texture);
                    ImGui::PushID(region.ID);

                    float buttonSize = itemWidth - 20.0f;

                    if (UI::ImageButton(texture, { buttonSize, buttonSize },
                        { region.UV.x, region.UV.y },
                        { region.UV.z, region.UV.w }))
                    {
                        m_SelectedRegion = (int)(&region - m_Atlas->Regions.data());
                        m_SelectedRegionID = region.ID;
                    }

                    float textWidth = ImGui::CalcTextSize(region.Name.c_str()).x;
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (buttonSize - textWidth) * 0.5f);
                    ImGui::Text(region.Name.c_str());

                    ImGui::PopID();
                    ImGui::EndGroup();

                    ImGui::NextColumn();
                }

                ImGui::Columns(1);
            }
        }

        ImGui::EndChild();

        if (ImGui::Button("Save", { 100.0f, lineHeight }))
        {
            m_Atlas->BuildRegionLookup();

            auto texpath           = std::filesystem::path(AssetManager::Get()->GetMetadata(m_Atlas->Texture).FilePath);
            auto path              = texpath.replace_extension(".ktatlas").string();
            TextureAtlasImporter::Save(m_Atlas, path);

            m_Atlas->Handle        = AssetHandle();
            AssetMetadata metadata = {};
            metadata.Type          = AssetType::TextureAtlas;
            metadata.FilePath      = path;

            AssetManager::Get()->ImportAsset(m_Atlas->Handle, metadata, m_Atlas);
            m_Active               = false;
        }
    }

    void TextureAtlasPanel::DrawImagePreview()
    {
        KTN_PROFILE_FUNCTION();

        ImGui::BeginGroup();

        ImGui::BeginChild("ImagePreview", ImVec2(0, 0), false);

        if (!m_Atlas->Texture)
        {
            ImVec2 size = ImGui::GetContentRegionAvail();

            ImGui::Dummy({ size.x * 0.3f, size.y * 0.3f });

            ImGui::SetCursorPosX((size.x - 120) * 0.5f);
            ImGui::Text("Drag a Image");

            ImGui::Spacing();

            ImGui::SetCursorPosX((size.x - 120) * 0.5f);
            if (ImGui::Button("Select Image", ImVec2(120, 0)))
            {
                std::string path = "";
                if (FileDialog::Open("*", "", path) == FileDialogResult::SUCCESS)
                {
                    m_Atlas->Texture = AssetManager::Get()->ImportAsset(AssetType::Texture2D, path);
                    GenerateUVs(m_Atlas, m_Grid);
                }
            }
        }
        else
        {
            auto texture              = AssetManager::Get()->GetAsset<Texture2D>(m_Atlas->Texture);
            if (texture)
            {
                ImVec2 availableSize  = ImGui::GetContentRegionAvail();

                ImVec2 textureSize    = { (float)texture->GetWidth(), (float)texture->GetHeight() };
                if (!m_Atlas->Regions.empty())
                {
                    auto region       = m_SelectedRegionID ? m_Atlas->GetRegion(m_SelectedRegionID) : &m_Atlas->Regions[0];
                    textureSize       = { region->GridSize.x, region->GridSize.y };
                }

                float aspectRatio     = textureSize.x / textureSize.y;
                ImVec2 imageSize      = availableSize;

                if (aspectRatio > 1.0f)
                {
                    imageSize.x       = availableSize.x;
                    imageSize.y       = availableSize.x / aspectRatio;

                    if (imageSize.y > availableSize.y)
                    {
                        imageSize.y   = availableSize.y;
                        imageSize.x   = availableSize.y * aspectRatio;
                    }
                }
                else
                {
                    imageSize.y       = availableSize.y;
                    imageSize.x       = availableSize.y * aspectRatio;

                    if (imageSize.x > availableSize.x)
                    {
                        imageSize.x   = availableSize.x;
                        imageSize.y   = availableSize.x / aspectRatio;
                    }
                }

                float posX = (availableSize.x - imageSize.x) * 0.5f;
                float posY = (availableSize.y - imageSize.y) * 0.5f;

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + posX);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + posY);

                if (!m_Atlas->Regions.empty())
                {
                    const auto& region = m_SelectedRegionID ? m_Atlas->GetRegion(m_SelectedRegionID) : &m_Atlas->Regions[0];
                    ImVec2 uv0 = { region->UV.x, region->UV.y };
                    ImVec2 uv1 = { region->UV.z, region->UV.w };
                    UI::Image(texture, imageSize, uv0, uv1);
                }
                else
                {
                    UI::Image(texture, imageSize);
                }
            }
        }

        ImGui::EndChild();
        ImGui::EndGroup();

        if (!m_Atlas->Texture && ImGui::BeginDragDropTarget())
        {
            if (auto payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                auto filepath = std::filesystem::path(path);

                m_Atlas->Texture = AssetManager::Get()->ImportAsset(AssetType::Texture2D, filepath.string());
                GenerateUVs(m_Atlas, m_Grid);
            }

            ImGui::EndDragDropTarget();
        }
    }

} // namespace KTN