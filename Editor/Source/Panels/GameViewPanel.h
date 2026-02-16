#pragma once
#include "EditorPanel.h"
#include "Koten/Graphics/Texture.h"

// lib
#include <imgui.h>



namespace KTN
{
    class GameViewPanel : public EditorPanel
    {
        public:
            GameViewPanel();
            ~GameViewPanel() override = default;

            void OnImgui() override;
            void OnUpdate() override;
            void OnRender() override;

        private:
            Ref<Texture2D> m_MainTexture = nullptr;
            uint32_t m_ViewportWidth     = 800;
            uint32_t m_ViewportHeight    = 600;
    };

} // namespace KTN
