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
            m_TitlebarHeight     = ImGui::GetFrameHeight();
            m_ViewportMinRegion  = ImGui::GetWindowContentRegionMin();
            m_ViewportMaxRegion  = ImGui::GetWindowContentRegionMax();
            m_ViewportOffset     = ImGui::GetWindowPos();

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

            auto* drawList = ImGui::GetWindowDrawList();
            ImVec2 min     = ImGui::GetItemRectMin();
            ImVec2 max     = ImGui::GetItemRectMax();

            drawList->AddRect(min, max, IM_COL32(255, 255, 255, 80));

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

            m_Width  = (uint32_t)imageSize.x;
            m_Height = (uint32_t)imageSize.y;

            glm::vec2 viewportBounds[2] = {
                { m_ViewportMinRegion.x + m_ViewportOffset.x, m_ViewportMinRegion.y + m_ViewportOffset.y },
                { m_ViewportMaxRegion.x + m_ViewportOffset.x, m_ViewportMaxRegion.y + m_ViewportOffset.y }
            };

            auto guizmoType = m_Editor->GetGuizmoType();

            //bool imguizmo = (guizmoType != 0 && ImGuizmo::IsOver()) || ImGuizmo::IsUsing();
            //if (Engine::Get().GetSettings().MousePicking && !ImGui::IsDragDropActive() && !imguizmo)
            //{
            //    auto [mx, my] = ImGui::GetMousePos();
            //    mx -= viewportBounds[0].x;
            //    my -= viewportBounds[0].y;
            //    if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
            //        my = m_Height - my;

            //    glm::ivec2 mouse = { (int)mx, (int)my };

            //    if ((mouse.x >= 0 && mouse.x < (int)m_Width) &&
            //        (mouse.y >= 0 && mouse.y < (int)m_Height))
            //    {
            //        if (Input::IsMouseButtonPressed(Mouse::Button_Left))
            //        {
            //            auto texture = Renderer::GetPickingTexture();
            //            int id = static_cast<int>((intptr_t)RendererCommand::ReadPixel(texture, mouse.x, mouse.y));
            //            if (id >= 0)
            //                m_Editor->SetSelectedEntt({ (entt::entity)id, m_Context.get() });
            //            else
            //                m_Editor->UnSelectEntt();
            //        }
            //    }
            //}


            // Gizmos
            Entity selectedEntity = m_Editor->GetSelected();
            if (selectedEntity && guizmoType != 0)
            {

                ImGuizmo::SetOrthographic(camera->GetMode() == EditorCameraMode::TWODIM);
                ImGuizmo::SetDrawlist();

                ImGuizmo::SetRect(viewportBounds[0].x, viewportBounds[0].y, viewportBounds[1].x - viewportBounds[0].x, viewportBounds[1].y - viewportBounds[0].y);

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
        tspec.Width                = m_Width;
        tspec.Height               = m_Height;
        tspec.Format               = TextureFormat::RGBA32_FLOAT;
        tspec.Usage                = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
        tspec.Samples              = 1;
        tspec.GenerateMips         = false;
        tspec.AnisotropyEnable     = false;
        tspec.DebugName            = "SceneView-MainTexture";

        m_MainTexture              = Texture2D::Get(tspec);

        auto& camera = m_Editor->GetCamera();
        camera->SetViewportSize(m_Width, m_Height);
        Application::Get().GetImGui()->BlockEvents(m_HandleCameraEvents);
        camera->SetHandleEvents(m_HandleCameraEvents);
    }

    void SceneViewPanel::OnRender()
    {
        KTN_PROFILE_FUNCTION();

        auto& camera = m_Editor->GetCamera();
        SceneManager::OnRender(m_MainTexture, m_Width, m_Height, camera->GetProjection(), camera->GetView());
    }
    
} // namespace KTN
