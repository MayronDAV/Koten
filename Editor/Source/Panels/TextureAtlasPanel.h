#pragma once
#include "Koten/Asset/Asset.h"
#include "EditorPanel.h"
#include "Koten/Asset/TextureAtlasImporter.h"



namespace KTN
{
    class TextureAtlasPanel : public EditorPanel
    {
        public:
            TextureAtlasPanel() : EditorPanel("TextureAtlas") { m_Active = false; }
            ~TextureAtlasPanel() override = default;

            void OnImgui() override;
            void Open();

        private:
            void DrawLeftPanel();
            void DrawImagePreview();

        private:
            Ref<TextureAtlas> m_Atlas          = nullptr;

            int m_SelectedRegion               = -1;
            uint64_t m_SelectedRegionID        = 0;
            glm::vec2 m_Grid                   = { 64.0f, 64.0f };
            glm::vec2 m_GridTemp               = { 64.0f, 64.0f };
    };

} // namespace KTN
