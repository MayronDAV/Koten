#pragma once
#include "EditorPanel.h"
#include "Koten/Graphics/Texture.h"

// lib
#include <imgui.h>



namespace KTN
{
    class SceneViewPanel : public EditorPanel
    {
        public:
            SceneViewPanel();
            ~SceneViewPanel() override = default;

            void OnImgui() override;
            void OnUpdate() override;
            void OnRender() override;

        private:
            Ref<Texture2D> m_MainTexture = nullptr;

            struct ViewportData
            {
                ImVec2 Position       = { 0.0f, 0.0f };
                ImVec2 Size           = { 0.0f, 0.0f };

                uint32_t RenderWidth  = 800;
                uint32_t RenderHeight = 600;
            };
            ViewportData m_Viewport;

            bool m_HandleCameraEvents    = false;
            uint32_t m_PickingTextureID  = 0;
    };

} // namespace KTN
