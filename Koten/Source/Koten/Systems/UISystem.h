#pragma once
#include "Koten/Core/Base.h"
#include "System.h"

#include "Koten/Graphics/RenderList.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/Entity.h"



namespace KTN
{
    class KTN_API UISystem : public System
    {
        public:
            bool OnStart(Scene* p_Scene);
            bool OnStop(Scene* p_Scene);

            void OnUpdate(Scene* p_Scene) override;
            void OnViewportUpdate(Scene* p_Scene, const Ref<ViewportContext>& p_Viewport) override;
            void OnViewportRender(Scene* p_Scene, const Ref<ViewportContext>& p_Viewport) override;

        private:
            void ProcessCanvas(Entity p_Canvas, const UICanvasComponent& p_CanvasComponent, const Ref<ViewportContext>& p_Viewport);

            struct CanvasRenderData
            {
                Entity Canvas;
                AssetHandle RenderTarget;
                uint32_t PickingTarget = 0;
                glm::vec2 RefResolution;
                glm::vec2 Scale;
                glm::vec2 Offset;
                RenderList List;
                bool ReceiveInput = true;

                int SortOrder;
            };

            void ProcessEntity(Entity p_Entity, CanvasRenderData& p_CanvasData, bool p_ParentActive = true);

        private:
            struct Data
            {
                glm::vec2 LastSize = { 0.0f, 0.0f };
                std::unordered_map<AssetHandle, std::vector<CanvasRenderData>> RenderData;
                std::unordered_map<uint64_t, uint32_t> PickingTargets;
            };

            std::unordered_map<ViewportID, Data> m_Data;
            bool m_IsRunning = false;
    };


} // namespace KTN
