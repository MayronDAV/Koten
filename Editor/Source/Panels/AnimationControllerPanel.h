#pragma once
#include "Koten/Asset/Asset.h"
#include "EditorPanel.h"
#include "Koten/Asset/AnimationControllerImporter.h"

// lib
#include <imgui_node_editor.h>



namespace KTN
{
    namespace axnd = ax::NodeEditor;

    struct AnimPin
    {
        uint64_t ID;
        uint64_t NodeId;
        axnd::PinKind Kind;
    };

    struct AnimNode
    {
        uint64_t ID     = 0;
        uint64_t ClipID = 0;
        std::string Name;
        bool IsEntry    = false;

        std::vector<uint64_t> Inputs;
        std::vector<uint64_t> Outputs;
    };

    struct AnimLink
    {
        uint64_t ID;
        uint64_t StartPin;
        uint64_t EndPin;
        uint64_t TransitionID;
    };

    class AnimationControllerPanel : public EditorPanel
    {
        public:
            AnimationControllerPanel() : EditorPanel("AnimationController") { m_Active = false; }
            ~AnimationControllerPanel() override { DestroyContext(); }

            void OnImgui() override;
            void Open();
            void Open(const std::string& p_Path);

        private:
            void DrawLeftPanel();
            void DrawRightPanel();
            void LoadPath(const std::string& p_Path);

            void CreateContext();
            void DestroyContext();
            void CreateEntryNode();

            void RemoveState(uint64_t p_StateID);
            void RemoveTransition(uint64_t p_LinkID);
            void DrawConditionsEditor(AnimationTransition& p_Transition);
            AnimPin* FindPin(uint64_t p_ID);

            void UpdateSelecteds();
            void RebuildGraphFromController();
            void UpdateInitialNodePosition(uint64_t p_NodeID);

        private:
            Ref<AnimationController> m_Controller     = nullptr;
            axnd::EditorContext* m_Context            = nullptr;

            std::vector<AnimNode> m_Nodes;
            std::vector<AnimPin>  m_Pins;
            std::vector<AnimLink> m_Links;

            std::vector<AnimationState*>       m_SelectedStates;
            std::vector<AnimationTransition*>  m_SelectedTransitions;

            uint64_t m_NextId                         = 1;
            float m_NextX                             = 0.0f;
            float m_NextY                             = 0.0f;
    };

} // namespace KTN
