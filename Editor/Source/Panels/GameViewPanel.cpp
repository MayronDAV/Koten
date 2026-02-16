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
    }

    void GameViewPanel::OnImgui()
    {
        KTN_PROFILE_FUNCTION();

        auto& camera               = m_Editor->GetCamera();
        glm::mat4 cameraProjection = camera->GetProjection();
        glm::mat4 cameraView       = camera->GetView();

        auto& config = Project::GetActive()->GetConfig(); // Width, Height this size is used for rendering just in the editor, not the viewport size

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 1.0f });
        ImGui::Begin(m_Name.c_str(), &m_Active);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        {
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();

            float targetAspect = (float)config.Width / (float)config.Height;
            float viewportAspect = viewportSize.x / viewportSize.y;

            ImVec2 imageSize;

            if (viewportAspect > targetAspect)
            {
                imageSize.y = viewportSize.y;
                imageSize.x = imageSize.y * targetAspect;
            }
            else
            {
                imageSize.x = viewportSize.x;
                imageSize.y = imageSize.x / targetAspect;
            }

            ImVec2 cursorPos = ImGui::GetCursorPos();
            ImGui::SetCursorPos({
                cursorPos.x + (viewportSize.x - imageSize.x) * 0.5f,
                cursorPos.y + (viewportSize.y - imageSize.y) * 0.5f
            });

            UI::Image(m_MainTexture, imageSize);

            auto* drawList = ImGui::GetWindowDrawList();
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();

            drawList->AddRect(min, max, IM_COL32(255, 255, 255, 80));

            m_ViewportWidth     = (uint32_t)imageSize.x;
            m_ViewportHeight    = (uint32_t)imageSize.y;
        }
        ImGui::End();
    }

    void GameViewPanel::OnUpdate()
    {
        KTN_PROFILE_FUNCTION();

        TextureSpecification tspec = {};
        tspec.Width                = m_ViewportWidth;
        tspec.Height               = m_ViewportHeight;
        tspec.Format               = TextureFormat::RGBA32_FLOAT;
        tspec.Usage                = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
        tspec.Samples              = 1;
        tspec.GenerateMips         = false;
        tspec.AnisotropyEnable     = false;
        tspec.DebugName            = "GameView-MainTexture";

        m_MainTexture              = Texture2D::Get(tspec);
    }

    void GameViewPanel::OnRender()
    {
        KTN_PROFILE_FUNCTION();

        SceneManager::OnRenderRuntime(m_MainTexture, m_ViewportWidth, m_ViewportHeight);
    }

} // namespace KTN