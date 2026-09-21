#include "ToolbarPanel.h"
#include "Editor.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>



namespace KTN
{
    namespace
    {
        static bool DrawSelectableIconButton(const char* p_Icon, bool p_Selected)
        {
            if (p_Selected)
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive]);

            bool pressed = ImGui::Button(p_Icon);

            if (p_Selected)
                ImGui::PopStyleColor();

            return pressed;
        }

        static float GetFPSWidth()
        {
            std::string text = std::format("FPS: {}", Engine::Get().GetStats().FramesPerSecond);
            float textWidth = ImGui::CalcTextSize(text.c_str()).x;
            float padding = ImGui::GetStyle().CellPadding.x * 2.0f;

            return textWidth + padding;
        }

        float GetGuizmoWidth()
        {
            float width = 0.0f;
            const float buttonWidth = ImGui::CalcTextSize(ICON_MDI_CURSOR_DEFAULT).x + ImGui::GetStyle().FramePadding.x * 2.0f;
            const float spacing = ImGui::GetStyle().ItemSpacing.x;
            int buttonCount = 5; // select, move, rotate, scale, universal
            width = buttonCount * buttonWidth + (buttonCount - 1) * spacing;

            return width + 20.0f; // + extra padding
        }

    } // namespace

    ToolbarPanel::ToolbarPanel()
        : EditorPanel("Toolbar")
    {
        m_Config.Dock = EditorPanelDock::Top;
    }

    void ToolbarPanel::OnImgui()
    {
        KTN_PROFILE_FUNCTION();

        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
        ImGui::SetNextWindowClass(&window_class);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        auto flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
        ImGui::Begin(m_Name.c_str(), &m_Active, flags);
        ImGui::PopStyleVar(3);

        ImVec2 size = ImGui::GetContentRegionAvail();

        if (ImGui::BeginTable("##ToolbarTable", 3, /*ImGuiTableFlags_NoBordersInBody*/ ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
        {
            float leftWidth = std::max(GetGuizmoWidth(), 100.0f);
            float rightWidth = std::max(GetFPSWidth(), 100.0f);

            ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthFixed, leftWidth);
            ImGui::TableSetupColumn("Center", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthFixed, rightWidth);

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            DrawGuizmoToolbar();

            ImGui::TableSetColumnIndex(1);
            DrawPlayControls();

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("FPS: %u", Engine::Get().GetStats().FramesPerSecond);

            ImGui::EndTable();
        }

        ImGui::End();
    }

    void ToolbarPanel::DrawGuizmoToolbar()
    {
        KTN_PROFILE_FUNCTION_LOW();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        bool selected = false;

        int guizmoType = m_Editor->GetGuizmoType();

        {
            selected = guizmoType == 0;
            if (DrawSelectableIconButton(ICON_MDI_CURSOR_DEFAULT, selected))
                m_Editor->SetGuizmoType(0);

            UI::Tooltip("Select");
        }
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        {
            selected = guizmoType == ImGuizmo::TRANSLATE;
            if (DrawSelectableIconButton(ICON_MDI_ARROW_ALL, selected))
                m_Editor->SetGuizmoType(ImGuizmo::TRANSLATE);

            UI::Tooltip("Translate");
        }

        ImGui::SameLine();

        {
            selected = guizmoType == ImGuizmo::ROTATE;
            if (DrawSelectableIconButton(ICON_MDI_ROTATE_ORBIT, selected))
                m_Editor->SetGuizmoType(ImGuizmo::ROTATE);

            UI::Tooltip("Rotate");
        }

        ImGui::SameLine();

        {
            selected = guizmoType == ImGuizmo::SCALE;
            if (DrawSelectableIconButton(ICON_MDI_ARROW_EXPAND_ALL, selected))
                m_Editor->SetGuizmoType(ImGuizmo::SCALE);

            UI::Tooltip("Scale");
        }

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        {
            selected = guizmoType == ImGuizmo::UNIVERSAL;
            if (DrawSelectableIconButton(ICON_MDI_CROP_ROTATE, selected))
                m_Editor->SetGuizmoType(ImGuizmo::UNIVERSAL);

            UI::Tooltip("Universal");
        }

        ImGui::PopStyleColor();
    }

    void ToolbarPanel::DrawPlayControls()
    {
        KTN_PROFILE_FUNCTION_LOW();

        float totalWidth = 0.0f;
        float spacing    = ImGui::GetStyle().ItemSpacing.x;

        auto calcButtonWidth = [](const char* icon)
        {
            return ImGui::CalcTextSize(icon).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        };

        auto state             = m_Editor->GetState();

        bool hasPlayButton     = state == RuntimeState::Edit || state == RuntimeState::Play;
        bool hasSimulateButton = state == RuntimeState::Edit || state == RuntimeState::Simulate;
        bool hasPauseButton    = state != RuntimeState::Edit;
        bool isPaused          = SceneManager::IsPaused();

        if (hasPlayButton)
            totalWidth += calcButtonWidth(state != RuntimeState::Play ? ICON_MDI_PLAY : ICON_MDI_STOP);

        if (hasSimulateButton)
        {
            if (totalWidth > 0) totalWidth += spacing;
            totalWidth += calcButtonWidth(state != RuntimeState::Simulate ? ICON_MDI_PLAY_BOX_OUTLINE : ICON_MDI_STOP);
        }

        if (hasPauseButton)
        {
            if (totalWidth > 0) totalWidth += spacing;
            totalWidth += calcButtonWidth(ICON_MDI_PAUSE);
        }

        if (isPaused)
        {
            if (totalWidth > 0) totalWidth += spacing;
            totalWidth += calcButtonWidth(ICON_MDI_STEP_FORWARD);
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));

        //float columnWidth = ImGui::GetColumnWidth();
        //ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - totalWidth) * 0.5f);

        float windowWidth = ImGui::GetWindowSize().x;
        float windowCenter = windowWidth * 0.5f;
        float windowPosX = ImGui::GetWindowPos().x;
        float cursorScreenX = ImGui::GetCursorScreenPos().x;
        float desiredScreenX = windowPosX + windowCenter - totalWidth * 0.5f;
        float offset = desiredScreenX - cursorScreenX;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);


        if (hasPlayButton)
        {
            std::string icon = state != RuntimeState::Play ? ICON_MDI_PLAY : ICON_MDI_STOP;
            if (ImGui::Button(icon.c_str()))
            {
                m_Editor->UnSelectEntt();
                if (state != RuntimeState::Play)
                    SceneManager::Play();
                else
                    SceneManager::Stop();

                m_Editor->SetState(state != RuntimeState::Play ? RuntimeState::Play : RuntimeState::Edit);
            }

            UI::Tooltip(state != RuntimeState::Play ? "Play" : "Stop");
        }

        if (hasSimulateButton)
        {
            if (hasPlayButton)
                ImGui::SameLine();

            // TODO: find an icon for simulate button
            std::string icon = state != RuntimeState::Simulate ? ICON_MDI_PLAY_BOX_OUTLINE : ICON_MDI_STOP;
            if (ImGui::Button(icon.c_str()))
            {
                if (state != RuntimeState::Simulate)
                    SceneManager::Simulate();
                else
                    SceneManager::Stop();
                m_Editor->SetState(state != RuntimeState::Simulate ? RuntimeState::Simulate : RuntimeState::Edit);
            }

            UI::Tooltip(state != RuntimeState::Simulate ? "Simulate" : "Stop");
        }

        if (hasPauseButton)
        {
            ImGui::SameLine();

            std::string icon = ICON_MDI_PAUSE;
            if (ImGui::Button(icon.c_str()))
            {
                SceneManager::Pause(!isPaused);
            }

            UI::Tooltip("Pause");
        }

        if (isPaused)
        {
            ImGui::SameLine();

            std::string icon = ICON_MDI_STEP_FORWARD;
            if (ImGui::Button(icon.c_str()))
            {
                SceneManager::Step();
            }

            UI::Tooltip("Step");
        }

        ImGui::PopStyleColor();
    }


} // namespace KTN
