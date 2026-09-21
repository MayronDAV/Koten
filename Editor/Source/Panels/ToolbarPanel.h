#pragma once
#include "EditorPanel.h"



namespace KTN
{
    class ToolbarPanel : public EditorPanel
    {
        public:
            ToolbarPanel();
            ~ToolbarPanel() override = default;

            void OnImgui() override;

        private:
            void DrawGuizmoToolbar();
            void DrawPlayControls();
    };

} // namespace KTN
