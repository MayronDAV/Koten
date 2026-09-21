#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Graphics/RenderList.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/Entity.h"



namespace KTN
{
    class KTN_API UISystem
    {
        public:
            void Init(Scene* p_Scene, const glm::vec2& p_LeftTop = glm::vec2(0.0f));
            void Update();
            void Render();
            void ProcessCanvas();

            void SetLeftTop(const glm::vec2& p_LeftTop) { m_LeftTop = p_LeftTop; }

            void Reset();

        private:
            void ProcessCanvas(Entity p_Canvas, const UICanvasComponent& p_CanvasComponent);

            struct CanvasRenderData
            {
                Entity Canvas;
                AssetHandle RenderTarget;
                uint32_t PickingTarget = 0;
                glm::vec2 RefResolution;
                glm::vec2 Scale;
                glm::vec2 Offset;
                RenderList List;

                int SortOrder;
            };

            void ProcessEntity(Entity p_Entity, CanvasRenderData& p_CanvasData, bool p_ParentActive = true);

        private:
            std::unordered_map<AssetHandle, std::vector<CanvasRenderData>> m_RenderData;
            std::unordered_map<uint64_t, uint32_t> m_PickingTargets;
            Scene* m_Scene = nullptr;
            glm::vec2 m_LeftTop = glm::vec2(0.0f);
    };


} // namespace KTN
