#include "ktnpch.h"
#include "UISystem.h"
#include "Koten/Graphics/PickingManager.h"
#include "Koten/Asset/AssetManager.h"
#include "Koten/Graphics/Renderer.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/Entity.h"
#include "Koten/OS/Input.h"
#include "Koten/OS/MouseCodes.h"
#include "Koten/Graphics/RendererCommand.h"



namespace KTN
{
    bool UISystem::OnStart(Scene* p_Scene)
    {
        m_IsRunning = true;

        return true;
    }

    bool UISystem::OnStop(Scene* p_Scene)
    {
        m_IsRunning = false;

        return true;
    }

    void UISystem::OnUpdate(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Scene || !m_IsRunning)
            return;

        auto& registry = p_Scene->GetRegistry();
        registry.view<RuntimeComponent, UIInputComponent>().each(
        [&](entt::entity p_Entity, const RuntimeComponent& p_Runtime, UIInputComponent& p_Input)
        {
            p_Input.Hovered = false;
            p_Input.Pressed = false;
        });
    }

    void UISystem::OnViewportUpdate(Scene* p_Scene, const Ref<ViewportContext>& p_Viewport)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Scene || !p_Viewport->UpdateUI)
            return;

        auto [it, inserted] = m_Data.try_emplace(p_Viewport->ID);
        if (!inserted && it == m_Data.end())
        {
            KTN_CORE_ERROR("Failed to updated the UI for the Viewport: {}", p_Viewport->ID);
            return;
        }
        auto& data            = it->second;

        data.RenderData.clear();
        auto& registry = p_Scene->GetRegistry();
        registry.view<RuntimeComponent, UICanvasComponent>().each(
        [&](entt::entity p_Entity, const RuntimeComponent& p_Runtime, const UICanvasComponent& p_Canvas)
        {
            if (!p_Runtime.Active)
                return;

            data.RenderData.try_emplace(p_Canvas.RenderTarget);

            Entity canvas(p_Entity, p_Scene);
            ProcessCanvas(canvas, p_Canvas, p_Viewport);
        });

        if (!Input::IsMouseInsideWindow() || !m_IsRunning || !p_Viewport->EnablePicking)
            return;

        for (auto& [handle, canvasList] : data.RenderData)
        {
            for (auto& canvasData : canvasList)
            {
                auto& canvasComp = canvasData.Canvas.GetComponent<UICanvasComponent>();
                if (!canvasComp.ReceiveInput) continue;

                uint64_t canvasID = canvasData.Canvas.GetUUID();

                bool hasPickingTarget = data.PickingTargets.find(canvasID) != data.PickingTargets.end();

                if (hasPickingTarget)
                {
                    glm::vec2 mousePos = p_Viewport->Position.length() != 0.0f ? Input::GetMousePosition() : Input::GetCursorPosition();

                    bool inside =
                        mousePos.x >= p_Viewport->Position.x &&
                        mousePos.x  < p_Viewport->Position.x + p_Viewport->Size.x &&
                        mousePos.y >= p_Viewport->Position.y &&
                        mousePos.y  < p_Viewport->Position.y + p_Viewport->Size.y;

                    if (inside)
                    {
                        float localX = mousePos.x - p_Viewport->Position.x;
                        float localY = mousePos.y - p_Viewport->Position.y;

                        int pixelX = (int)localX;
                        int pixelY = (int)localY;

                        if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
                            pixelY = p_Viewport->Size.y - 1 - pixelY;

                        Entity entity = PickingManager::ReadPixel(data.PickingTargets[canvasID], pixelX, pixelY);
                        if (entity)
                        {
                            KTN_CORE_WARN("Pixel Position: ({}, {})", pixelX, pixelY);
                            auto* inputComponent = entity.TryGetComponent<UIInputComponent>();
                            if (inputComponent)
                            {
                                inputComponent->Hovered = true;
                                KTN_CORE_INFO("Hovered Entity: {}", (uint64_t)entity.GetUUID());
                                if (Input::IsMouseButtonPressed(Mouse::Button_Left))
                                {
                                    inputComponent->Pressed = true;
                                    KTN_CORE_INFO("Pressed Entity: {}", (uint64_t)entity.GetUUID());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void UISystem::OnViewportRender(Scene* p_Scene, const Ref<ViewportContext>& p_Viewport)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Scene || !p_Viewport->RenderUI)
            return;

        auto& data                 = m_Data[p_Viewport->ID];

        for (const auto& [handle, canvasList] : data.RenderData)
        {
            RenderPassInfo info    = {};
            info.RenderTarget      = p_Viewport->RenderTarget;
            info.Width             = (uint32_t)p_Viewport->Size.x;
            info.Height            = (uint32_t)p_Viewport->Size.y;
            info.Picking           = p_Viewport->EnablePicking;

            if (p_Viewport->HasCustomCamera())
            {
                auto& data         = p_Viewport->GetCustomCamera();
                info.ClearColor    = data.ClearColor;
            }
            else if (p_Viewport->HasSceneCameras())
            {
                auto& data         = p_Viewport->GetSceneCameras()[p_Scene->Handle];
                info.ClearColor    = data.ClearColor;
            }

            for (auto& canvas : canvasList)
            {
                if (canvas.ReceiveInput && p_Viewport->EnablePicking)
                {
                    uint64_t canvasID                 = canvas.Canvas.GetUUID();
                    if (data.PickingTargets.find(canvasID) == data.PickingTargets.end())
                        data.PickingTargets[canvasID] = PickingManager::CreatePickingTarget(info.Width, info.Height);
                    else
                        PickingManager::Update(data.PickingTargets[canvasID], info.Width, info.Height);

                    info.PickingTarget                = PickingManager::GetPickingTarget(data.PickingTargets[canvasID]);
                }

                info.Projection    = glm::ortho(0.0f, p_Viewport->Size.x, p_Viewport->Size.y, 0.0f, -1.0f, 1.0f);
                info.View          = glm::mat4(1.0f);
                info.Clear         = true;

                Renderer::BeginPass(info);
                {
                    Renderer::Submit(canvas.List);
                }
                Renderer::EndPass();
            }
        }
    }

    void UISystem::ProcessCanvas(Entity p_Canvas, const UICanvasComponent& p_CanvasComponent, const Ref<ViewportContext>& p_Viewport)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Canvas.IsActive())
            return;

        auto* hierarchy            = p_Canvas.TryGetComponent<HierarchyComponent>();
        if (!hierarchy || hierarchy->First == entt::null)
            return;

        auto& data                 = m_Data[p_Viewport->ID];

        auto& canvas               = data.RenderData[p_CanvasComponent.RenderTarget].emplace_back();
        canvas.Canvas              = p_Canvas;
        canvas.RefResolution       = p_CanvasComponent.ReferenceResolution;
        canvas.RenderTarget        = p_CanvasComponent.RenderTarget;
        canvas.SortOrder           = p_CanvasComponent.SortOrder;
        canvas.ReceiveInput        = p_CanvasComponent.ReceiveInput;

        glm::vec2 scale            = { p_Viewport->Size.x / canvas.RefResolution.x, p_Viewport->Size.y / canvas.RefResolution.y };
        if (p_CanvasComponent.ScaleMode == UIScaleMode::Fit)
        {
            float uniformScale     = std::min(scale.x, scale.y);
            canvas.Scale           = { uniformScale, uniformScale };
        }
        else if (p_CanvasComponent.ScaleMode == UIScaleMode::Fill)
        {
            float uniformScale     = std::max(scale.x, scale.y);
            canvas.Scale           = { uniformScale, uniformScale };
        }
        else
        {
            canvas.Scale           = scale;
        }

        glm::vec2 scaledResolution = canvas.RefResolution * canvas.Scale;
        canvas.Offset              = (p_Viewport->Size - scaledResolution) * 0.5f;

        auto scene                 = p_Canvas.GetScene();
        auto& registry             = scene->GetRegistry();
        entt::entity child         = hierarchy->First;

        while (child != entt::null && registry.valid(child))
        {
            Entity childEntity(child, scene);

            ProcessEntity(childEntity, canvas, true);

            auto* childHierarchy = childEntity.TryGetComponent<HierarchyComponent>();
            child                = childHierarchy ? childHierarchy->Next : entt::null;
        }
    }

    void UISystem::ProcessEntity(Entity p_Entity, CanvasRenderData& p_CanvasData, bool p_ParentActive)
    {
        KTN_PROFILE_FUNCTION();

        auto* uiComponent                      = p_Entity.TryGetComponent<UIComponent>();
        bool active                            = p_ParentActive && (uiComponent && uiComponent->Active) && p_Entity.IsActive();

        if (active)
        {
            RenderCommand command              = {};
            command.ID                         = INVALID_PICKING_ID;
            auto* inputComponent               = p_Entity.TryGetComponent<UIInputComponent>();
            if (inputComponent)
            {
                command.ID                     = PickingManager::RegisterEntity(p_Entity, false);
            }

            glm::vec2 logicalPosition          = uiComponent->Anchor * p_CanvasData.RefResolution;

            Math::Transform transform({ logicalPosition * p_CanvasData.Scale + p_CanvasData.Offset, 0.0f });
            glm::vec3 size                     = { uiComponent->Size * p_CanvasData.Scale, 1.0f };
            auto* enttTransform                = p_Entity.TryGetComponent<TransformComponent>();
            if (enttTransform)
                transform.SetLocalScale(size * enttTransform->GetLocalScale());
            else
                transform.SetLocalScale(size);

            command.Transform                  = transform.GetLocalMatrix();

            auto* imageComponent               = p_Entity.TryGetComponent<UIImageComponent>();
            if (imageComponent)
            {
                SpriteCommand spriteCommand    = {};
                spriteCommand.Type             = RenderType2D::Quad;
                spriteCommand.Size             = { 0.0f, 0.0f };
                spriteCommand.BySize           = true;
                spriteCommand.Offset           = { 0.0f, 0.0f };
                spriteCommand.Scale            = { 1.0f, -1.0f };
                spriteCommand.UseDirectUVs     = false;

                if (imageComponent->Type == UIImageComponent::ImageType::Material)
                {
                    auto material              = AssetManager::Get()->GetAsset<Material>(imageComponent->Handle);
                    if (material)
                    {
                        spriteCommand.Color    = material->AlbedoColor;
                        spriteCommand.Texture  = AssetManager::Get()->GetAsset<Texture2D>(material->Texture);
                        command.Command        = spriteCommand;

                        p_CanvasData.List.Submit(command);
                    }
                }
                else
                {
                    auto image                 = AssetManager::Get()->GetAsset<Texture2D>(imageComponent->Handle);
                    if (image)
                    {
                        spriteCommand.Color    = { 1.0f, 1.0f, 1.0f, 1.0f };
                        spriteCommand.Texture  = image;
                        command.Command        = spriteCommand;

                        p_CanvasData.List.Submit(command);
                    }
                }
            }
        }

        auto* hierarchy    = p_Entity.TryGetComponent<HierarchyComponent>();
        if (!hierarchy || hierarchy->First == entt::null)
            return;

        auto scene         = p_Entity.GetScene();
        auto& registry     = scene->GetRegistry();
        entt::entity child = hierarchy->First;

        while (child != entt::null && registry.valid(child))
        {
            Entity childEntity(child, scene);

            ProcessEntity(childEntity, p_CanvasData, active);

            auto* childHierarchy = childEntity.TryGetComponent<HierarchyComponent>();
            child                = childHierarchy ? childHierarchy->Next : entt::null;
        }
    }


} // namespace KTN
