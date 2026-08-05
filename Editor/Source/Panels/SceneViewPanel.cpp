#include "SceneViewPanel.h"
#include "Editor.h"
#include "Shortcuts.h"
#include "AssetImporterPanel.h"

// lib
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>




namespace KTN
{

    SceneViewPanel::SceneViewPanel()
        : EditorPanel("Scene View")
    {
        m_PickingTextureID = PickingManager::CreatePickingTarget(m_Viewport.RenderWidth, m_Viewport.RenderHeight);
    }

    void SceneViewPanel::OnImgui()
    {
        KTN_PROFILE_FUNCTION();

        auto& camera               = m_Editor->GetCamera();
        glm::mat4 cameraProjection = camera->GetProjection();
        glm::mat4 cameraView       = camera->GetView();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
        ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 1.0f });
        ImGui::Begin(m_Name.c_str(), &m_Active);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        {
            ImVec2 viewportSize  = ImGui::GetContentRegionAvail();
            m_HandleCameraEvents = ImGui::IsWindowFocused();

            float targetAspect   = 2.33f;
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

            auto* drawList          = ImGui::GetWindowDrawList();
            ImVec2 imageMin         = ImGui::GetItemRectMin();
            ImVec2 imageMax         = ImGui::GetItemRectMax();

            m_Viewport.Position     = imageMin;
            m_Viewport.Size         = imageSize;
            m_Viewport.RenderWidth  = (uint32_t)imageSize.x;
            m_Viewport.RenderHeight = (uint32_t)imageSize.y;

            drawList->AddRect(imageMin, imageMax, IM_COL32(255, 255, 255, 80));

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    SceneManager::Stop();

                    const wchar_t* path = (const wchar_t*)payload->Data;
                    auto filepath = std::filesystem::path(path);
                    if (filepath.extension() == ".ktscn")
                    {
                        SceneManager::Load(AssetManager::Get()->ImportAsset(AssetType::Scene, filepath.string()), LoadMode::Single);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            auto guizmoType = m_Editor->GetGuizmoType();

            bool imguizmo = (guizmoType != 0 && ImGuizmo::IsOver()) || ImGuizmo::IsUsing();
            if (Engine::Get().GetSettings().MousePicking && !ImGui::IsDragDropActive() && !imguizmo)
            {
                ImVec2 mousePos  = ImGui::GetMousePos();

                bool inside      =
                    mousePos.x >= imageMin.x &&
                    mousePos.x <  imageMax.x &&
                    mousePos.y >= imageMin.y &&
                    mousePos.y <  imageMax.y;

                if (inside && Input::IsMouseButtonPressed(Mouse::Button_Left))
                {
                    float localX = mousePos.x - imageMin.x;
                    float localY = mousePos.y - imageMin.y;

                    int pixelX   = (int)localX;
                    int pixelY   = (int)localY;

                    if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
                        pixelY   = m_Viewport.RenderHeight - 1 - pixelY;

                    auto entity  = PickingManager::ReadPixel(m_PickingTextureID, pixelX, pixelY);
                    m_Editor->SetSelectedEntt(entity);
                }
            }

            // Gizmos
            Entity selectedEntity = m_Editor->GetSelected();
            if (selectedEntity && guizmoType != 0)
            {
                ImGuizmo::SetOrthographic(camera->GetMode() == EditorCameraMode::TWODIM);
                ImGuizmo::SetDrawlist();

                ImGuizmo::SetRect(imageMin.x, imageMin.y, imageMax.x - imageMin.x, imageMax.y - imageMin.y);

                auto tc = selectedEntity.TryGetComponent<TransformComponent>();
                if (tc)
                {
                    glm::mat4 transform = tc->GetLocalMatrix();

                    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                        (ImGuizmo::OPERATION)guizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform));

                    if (ImGuizmo::IsUsing())
                    {
                        glm::vec3 translation, rotation, scale;
                        Math::Transform::Decompose(transform, translation, scale, rotation);

                        tc->SetLocalTranslation(translation);
                        tc->SetLocalRotation(rotation);
                        tc->SetLocalScale(scale);
                    }
                }
            }
        }
        ImGui::End();
    }

    void SceneViewPanel::OnUpdate()
    {
        KTN_PROFILE_FUNCTION();

        TextureSpecification tspec = {};
        tspec.Width                = m_Viewport.RenderWidth;
        tspec.Height               = m_Viewport.RenderHeight;
        tspec.Format               = TextureFormat::RGBA32_FLOAT;
        tspec.Usage                = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
        tspec.Samples              = 1;
        tspec.GenerateMips         = false;
        tspec.AnisotropyEnable     = false;
        tspec.DebugName            = "SceneView-MainTexture";

        m_MainTexture              = Texture2D::Get(tspec);

        PickingManager::Update(m_PickingTextureID, m_Viewport.RenderWidth, m_Viewport.RenderHeight);

        auto& camera               = m_Editor->GetCamera();
        camera->SetViewportSize(m_Viewport.RenderWidth, m_Viewport.RenderHeight);
        Application::Get().GetImGui()->BlockEvents(m_HandleCameraEvents);
        camera->SetHandleEvents(m_HandleCameraEvents);
    }

    void SceneViewPanel::OnRender()
    {
        KTN_PROFILE_FUNCTION();

        auto& camera               = m_Editor->GetCamera();
        SceneManager::SetPickingTarget(PickingManager::GetPickingTarget(m_PickingTextureID));
        SceneManager::OnRender(m_MainTexture, m_Viewport.RenderWidth, m_Viewport.RenderHeight, camera->GetProjection(), camera->GetView());
        SceneManager::SetPickingTarget(nullptr);
    }

} // namespace KTN
