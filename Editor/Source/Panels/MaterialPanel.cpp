#include "MaterialPanel.h"
#include "Editor.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>



namespace KTN
{
    namespace
    {
        static std::string GetRelative(const std::string& p_Path)
        {
            auto relativePath = FileSystem::GetRelative(p_Path, Project::GetActive()->GetAssetDirectory().string());
            if (relativePath.empty())
                return p_Path;

            return relativePath;
        }

    } // namespace

    MaterialPanel::MaterialPanel()
        : EditorPanel("Material")
    {
        m_Active = false;
    }

    void MaterialPanel::Open(AssetHandle p_Material)
    {
        m_Active = true;
        m_Changed = true;
        m_MaterialHandle = p_Material;
        m_Material = p_Material ? AssetManager::Get()->GetAsset<Asset>(m_MaterialHandle) : nullptr;
    }

    void MaterialPanel::OnImgui()
    {
        KTN_PROFILE_FUNCTION();

        ImGui::Begin(m_Name.c_str(), &m_Active);

        ImGui::BeginGroup();

        if (m_Changed)
        {
            m_Material = m_MaterialHandle ? AssetManager::Get()->GetAsset<Asset>(m_MaterialHandle) : nullptr;
            m_Metadata = AssetManager::Get()->GetMetadata(m_MaterialHandle);
            m_Changed = false;
        }

        ImGui::Text("Path");
        if (UI::InputText("#Path", m_Metadata.FilePath, false, 0, 2.0f, true))
            m_Edited = true;

        ImVec2 btnPos = ImGui::GetCursorScreenPos();
        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 btnSize = ImVec2(100.0f, lineHeight);

        if (ImGui::Button(ICON_MDI_PLUS_BOX " New", btnSize))
        {
            m_MaterialHandle = AssetHandle();
            m_New = true;
            ImGui::OpenPopup("NewDropdownPopup");
        }

        ImGui::SetNextWindowPos(
            ImVec2(btnPos.x + btnSize.x + 4, btnPos.y),
            ImGuiCond_Appearing
        );

        if (ImGui::BeginPopup("NewDropdownPopup"))
        {
            if (ImGui::Selectable("Copy"))
            {
                m_Metadata.FilePath = (Project::GetAssetFileSystemPath("Materials") / (FileSystem::GetStem(m_Metadata.FilePath) + (m_Metadata.Type == AssetType::Material ? "(1).ktmat" : "(1).ktasset"))).string();
                m_Material->Handle  = m_MaterialHandle;
            }

            if (ImGui::Selectable("Material"))
            {
                m_Metadata.Type     = AssetType::Material;
                m_Metadata.FilePath = (Project::GetAssetFileSystemPath("Materials") / "New.ktmat").string();

                auto mat            = CreateRef<Material>();
                mat->Name           = "New Material";

                m_Material          = mat;
                m_Material->Handle  = m_MaterialHandle;
            }

            if (ImGui::Selectable("PhysicsMaterial2D"))
            {
                m_Metadata.Type     = AssetType::PhysicsMaterial2D;
                m_Metadata.FilePath = (Project::GetAssetFileSystemPath("Materials") / "New.ktasset").string();
                m_Material          = CreateRef<PhysicsMaterial2D>();
                m_Material->Handle  = m_MaterialHandle;
            }

            ImGui::EndPopup();
        }

        auto size = ImGui::GetContentRegionAvail();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        ImGui::BeginChild("MaterialOptions", { size.x, size.y - (lineHeight * 1.2f) }, false);
        ImGui::PopStyleVar();
        if (m_Material)
        {
            auto type = m_Metadata.Type;
            if (type == AssetType::Material)
            {
                auto material = As<Asset, Material>(m_Material);

                UI::ColorEdit4("Color", material->AlbedoColor, 1.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

                if (ImGui::Button(ICON_MDI_CANCEL))
                    material->Texture = Texture2D::GetDefault();

                ImGui::SameLine();

                std::string path = material->Texture != 0 ? GetRelative(AssetManager::Get()->GetMetadata(material->Texture).FilePath) : "";
                std::string text = path.empty() ? "Texture" : path.c_str();
                if (ImGui::Button(text.c_str()))
                {
                    std::string path = "";
                    if (FileDialog::Open("", Project::GetAssetDirectory().string(), path) == FileDialogResult::SUCCESS)
                    {
                        auto filepath = std::filesystem::path(path);
                        if (filepath.extension() == ".png" || filepath.extension() == ".jpg" || filepath.extension() == ".jpeg")
                            material->Texture = AssetManager::Get()->ImportAsset(AssetType::Texture2D, path);
                    }
                }

                ImGui::PopStyleVar();

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        auto filepath = std::filesystem::path(path);
                        if (filepath.extension() == ".png" || filepath.extension() == ".jpg" || filepath.extension() == ".jpeg")
                            material->Texture = AssetManager::Get()->ImportAsset(AssetType::Texture2D, filepath.string());
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            if (type == AssetType::PhysicsMaterial2D)
            {
                auto material = As<Asset, PhysicsMaterial2D>(m_Material);

                ImGui::InputFloat("Friction", &material->Friction);
                ImGui::InputFloat("Restitution", &material->Restitution);
                ImGui::InputFloat("RestitutionThreshold", &material->RestitutionThreshold);
            }
        }
        ImGui::EndChild();

        if (ImGui::Button("Save", { 100.0f, lineHeight }) && m_Material)
        {
            if (m_Edited)
            {
                if (AssetManager::Get()->IsAssetHandleValid(m_MaterialHandle))
                {
                    auto& metadata = AssetManager::Get()->GetMetadata(m_MaterialHandle);
                    metadata.FilePath = m_Metadata.FilePath;
                    AssetManager::Get()->SerializeAssetRegistry();
                }
                m_Edited = false;
            }

            if (m_New)
            {
                AssetManager::Get()->ImportAsset(m_MaterialHandle, m_Metadata, m_Material);
                m_New = false;
            }

            if (m_Metadata.Type == AssetType::Material)
            {
                auto material = As<Asset, Material>(m_Material);
                material->Serialize(m_Metadata.FilePath);
            }

            if (m_Metadata.Type == AssetType::PhysicsMaterial2D)
            {
                auto material = As<Asset, PhysicsMaterial2D>(m_Material);
                material->Serialize(m_Metadata.FilePath);
            }
        }

        ImGui::EndGroup();

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                auto filepath = std::filesystem::path(path);
                if (filepath.extension() == ".ktasset")
                {
                    m_MaterialHandle = AssetManager::Get()->ImportAsset(AssetType::PhysicsMaterial2D, filepath.string());
                    m_Changed = true;
                }

                if (filepath.extension() == ".ktmat")
                {
                    m_MaterialHandle = AssetManager::Get()->ImportAsset(AssetType::Material, filepath.string());
                    m_Changed = true;
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::End();

    }


} // namespace KTN
