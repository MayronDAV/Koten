#include "GameViewPanel.h"
#include "Editor.h"

// lib
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>



namespace KTN
{
    GameViewPanel::GameViewPanel()
        : EditorPanel("Game View")
    {
        m_Config.Dock = EditorPanelDock::Down;
    }

    void GameViewPanel::OnImgui()
    {
        KTN_PROFILE_FUNCTION();

        auto& config = Project::GetActive()->GetConfig(); // Width, Height this size is used for rendering just in the editor, not the viewport size

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 1.0f });
        ImGui::Begin(m_Name.c_str(), &m_Active);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        {
            ImVec2 viewportSize  = ImGui::GetContentRegionAvail();

            float targetAspect   = (float)config.Width / (float)config.Height;
            float viewportAspect = viewportSize.x / viewportSize.y;

            ImVec2 imageSize;

            if (viewportAspect > targetAspect)
            {
                imageSize.y      = viewportSize.y;
                imageSize.x      = imageSize.y * targetAspect;
            }
            else
            {
                imageSize.x      = viewportSize.x;
                imageSize.y      = imageSize.x / targetAspect;
            }

            ImVec2 cursorPos     = ImGui::GetCursorPos();
            ImGui::SetCursorPos({
                cursorPos.x + (viewportSize.x - imageSize.x) * 0.5f,
                cursorPos.y + (viewportSize.y - imageSize.y) * 0.5f
            });

            UI::Image(m_MainTexture, imageSize);

            auto* drawList       = ImGui::GetWindowDrawList();
            ImVec2 min           = ImGui::GetItemRectMin();
            ImVec2 max           = ImGui::GetItemRectMax();

            drawList->AddRect(min, max, IM_COL32(255, 255, 255, 80));

            m_Viewport.Position  = min;
            m_Viewport.Width     = (uint32_t)imageSize.x;
            m_Viewport.Height    = (uint32_t)imageSize.y;
        }
        ImGui::End();
    }

    void GameViewPanel::OnUpdate()
    {
        KTN_PROFILE_FUNCTION();

        TextureSpecification tspec  = {};
        tspec.Width                 = m_Viewport.Width;
        tspec.Height                = m_Viewport.Height;
        tspec.Format                = TextureFormat::RGBA32_FLOAT;
        tspec.Usage                 = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
        tspec.Samples               = 1;
        tspec.GenerateMips          = false;
        tspec.AnisotropyEnable      = false;
        tspec.DebugName             = "GameView-MainTexture";

        m_MainTexture               = Texture2D::Get(tspec);

        auto viewport               = SceneManager::GetOrCreateViewport("GameViewport");
        viewport->RenderTarget      = nullptr;
        viewport->RenderTarget      = m_MainTexture;
        viewport->PickingTarget     = nullptr;

        viewport->EnablePicking     = false;

        auto state = m_Editor->GetState();
        if (state != RuntimeState::Edit)
        {
            if (m_PickingTextureID == 0)
                m_PickingTextureID  = PickingManager::CreatePickingTarget(m_Viewport.Width, m_Viewport.Height);

            PickingManager::Update(m_PickingTextureID, m_Viewport.Width, m_Viewport.Height);
            viewport->PickingTarget = PickingManager::GetPickingTarget(m_PickingTextureID);
            viewport->EnablePicking = true;
        }

        viewport->Size              = { (float)m_Viewport.Width, (float)m_Viewport.Height };
        viewport->Position          = { m_Viewport.Position.x, m_Viewport.Position.y };

        viewport->SetSceneCameras();

        viewport->UpdateUI          = true;
        viewport->RenderUI          = true;
    }

} // namespace KTN