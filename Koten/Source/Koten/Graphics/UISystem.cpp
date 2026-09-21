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
    void UISystem::Init(Scene* p_Scene, const glm::vec2& p_LeftTop)
    {
        KTN_PROFILE_FUNCTION();

        KTN_CORE_ASSERT(p_Scene, "Scene is null!");
        m_Scene   = p_Scene;
        m_LeftTop = p_LeftTop;
    }

    void UISystem::Update()
    {
        KTN_PROFILE_FUNCTION();

        if (!m_Scene)
            return;

        m_RenderData.clear();
        auto& registry = m_Scene->GetRegistry();
        registry.view<RuntimeComponent, UICanvasComponent>().each(
        [&](entt::entity p_Entity, const RuntimeComponent& p_Runtime, const UICanvasComponent& p_Canvas)
        {
            if (!p_Runtime.Active)
                return;

            m_RenderData.try_emplace(p_Canvas.RenderTarget);

            Entity canvas(p_Entity, m_Scene);
            ProcessCanvas(canvas, p_Canvas);
        });
    }

    void UISystem::Render()
    {
        KTN_PROFILE_FUNCTION();

        for (const auto& [handle, canvasList] : m_RenderData)
        {
            auto renderTarget      = handle ? AssetManager::Get()->GetAsset<Texture2D>(handle) : m_Scene->GetRenderTarget();
            if (!renderTarget)
                continue;

            RenderPassInfo info    = {};
            info.RenderTarget      = renderTarget;
            info.Width             = renderTarget->GetWidth();
            info.Height            = renderTarget->GetHeight();
            info.Picking           = m_Scene->GetPickingTarget() != nullptr;

            for (auto& canvas : canvasList)
            {
                if (info.Picking)
                {
                    uint64_t canvasID = canvas.Canvas.GetUUID();
                    if (m_PickingTargets.find(canvasID) == m_PickingTargets.end())
                        m_PickingTargets[canvasID] = PickingManager::CreatePickingTarget(info.Width, info.Height);

                    info.PickingTarget = PickingManager::GetPickingTarget(m_PickingTargets[canvasID]);
                }

                info.Projection    = glm::ortho(0.0f, (float)info.Width, (float)info.Height, 0.0f, -1.0f, 1.0f);
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

    void UISystem::ProcessCanvas()
    {
        KTN_PROFILE_FUNCTION();

        if (!Input::IsMouseInsideWindow())
            return;

        for (auto& [handle, canvasList] : m_RenderData)
        {
            for (auto& canvasData : canvasList)
            {
                auto& canvasComp = canvasData.Canvas.GetComponent<UICanvasComponent>();
                if (!canvasComp.ReceiveInput) continue;

                uint64_t canvasID = canvasData.Canvas.GetUUID();

                bool hasPickingTarget = m_PickingTargets.find(canvasID) != m_PickingTargets.end();

                if (hasPickingTarget)
                {
                    glm::vec2 mousePos = Input::GetMousePosition();
                    auto renderTarget = canvasComp.RenderTarget ? AssetManager::Get()->GetAsset<Texture2D>(canvasComp.RenderTarget) : m_Scene->GetRenderTarget();
                    if (!renderTarget)
                        return;

                    float width = (float)renderTarget->GetWidth();
                    float height = (float)renderTarget->GetHeight();

                    bool inside =
                        mousePos.x >= m_LeftTop.x &&
                        mousePos.x < m_LeftTop.x + width &&
                        mousePos.y >= m_LeftTop.y &&
                        mousePos.y < m_LeftTop.y + height;

                    if (inside)
                    {
                        float localX = mousePos.x - m_LeftTop.x;
                        float localY = mousePos.y - m_LeftTop.y;

                        int pixelX = (int)localX;
                        int pixelY = (int)localY;

                        if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
                            pixelY = height - 1 - pixelY;

                        Entity entity = PickingManager::ReadPixel(m_PickingTargets[canvasID], pixelX, pixelY);
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

    void UISystem::Reset()
    {
        m_Scene->GetRegistry().view<RuntimeComponent, UIInputComponent>().each(
        [&](entt::entity p_Entity, const RuntimeComponent& p_Runtime, UIInputComponent& p_Input)
        {
            if (!p_Runtime.Active)
                return;

            p_Input.Hovered = false;
            p_Input.Pressed = false;
        });
    }

    void UISystem::ProcessCanvas(Entity p_Canvas, const UICanvasComponent& p_CanvasComponent)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Canvas.IsActive())
            return;

        auto* hierarchy            = p_Canvas.TryGetComponent<HierarchyComponent>();
        if (!hierarchy || hierarchy->First == entt::null)
            return;

        auto renderTarget          = p_CanvasComponent.RenderTarget ? AssetManager::Get()->GetAsset<Texture2D>(p_CanvasComponent.RenderTarget) : m_Scene->GetRenderTarget();
        if (!renderTarget)
            return;

        auto& canvas               = m_RenderData[p_CanvasComponent.RenderTarget].emplace_back();
        canvas.Canvas              = p_Canvas;
        canvas.RefResolution       = p_CanvasComponent.ReferenceResolution;
        canvas.RenderTarget        = p_CanvasComponent.RenderTarget;
        canvas.SortOrder           = p_CanvasComponent.SortOrder;

        glm::vec2 targetResolution = { (float)renderTarget->GetWidth(), (float)renderTarget->GetHeight() };
        glm::vec2 scale            = { targetResolution.x / canvas.RefResolution.x, targetResolution.y / canvas.RefResolution.y };
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
        canvas.Offset              = (targetResolution - scaledResolution) * 0.5f;

        auto& registry       = m_Scene->GetRegistry();
        entt::entity child   = hierarchy->First;

        while (child != entt::null && registry.valid(child))
        {
            Entity childEntity(child, m_Scene);

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
            auto* imageComponent               = p_Entity.TryGetComponent<UIImageComponent>();
            if (imageComponent)
            {
                RenderCommand command          = {};
                command.ID                     =  p_Entity.HasComponent<UIInputComponent>() ? PickingManager::RegisterEntity(p_Entity, false) : INVALID_PICKING_ID;

                glm::vec2 logicalPosition      = uiComponent->Anchor * p_CanvasData.RefResolution;

                Math::Transform transform({ logicalPosition * p_CanvasData.Scale + p_CanvasData.Offset, 0.0f });
                glm::vec3 size                 = { uiComponent->Size * p_CanvasData.Scale, 1.0f };
                auto* enttTransform            = p_Entity.TryGetComponent<TransformComponent>();
                if (enttTransform)
                    transform.SetLocalScale(size * enttTransform->GetLocalScale());
                else
                    transform.SetLocalScale(size);

                command.Transform              = transform.GetLocalMatrix();

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

        auto& registry     = m_Scene->GetRegistry();
        entt::entity child = hierarchy->First;

        while (child != entt::null && registry.valid(child))
        {
            Entity childEntity(child, m_Scene);

            ProcessEntity(childEntity, p_CanvasData, active);

            auto* childHierarchy = childEntity.TryGetComponent<HierarchyComponent>();
            child                = childHierarchy ? childHierarchy->Next : entt::null;
        }
    }


} // namespace KTN
