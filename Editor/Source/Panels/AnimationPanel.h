#pragma once
#include "Koten/Asset/Asset.h"
#include "EditorPanel.h"
#include "Koten/Asset/AnimationImporter.h"



namespace KTN
{
    class AnimationPanel : public EditorPanel
    {
    public:
        AnimationPanel() : EditorPanel("Animation") { m_Active = false; }
        ~AnimationPanel() override = default;

        void OnImgui() override;
        void Open();
        void Open(const std::string& p_Path);

    private:
        void DrawLeftPanel();
        void DrawRightPanel();
        void LoadPath(const std::string& p_Path);

    private:
        Ref<Animation> m_Anim       = nullptr;

        uint64_t m_SelectedClipID   = 0;
        std::string m_AnimationName = "New Animation";
    };

} // namespace KTN
