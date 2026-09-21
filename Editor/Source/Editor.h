#pragma once
#include "Koten/Koten.h"
#include "EditorPanel.h"
#include "EditorCamera.h"


namespace KTN
{
    class SettingsPanel;
    class AssetImporterPanel;
    class ProjectExporterPanel;
    class MaterialPanel;
    class SceneEditPanel;
    class TextureAtlasPanel;
    class AnimationPanel;
    class AnimationControllerPanel;

    enum class RuntimeState
    {
        Edit,
        Play,
        Simulate
    };

    struct LayoutConfig
    {
        float TopRatio   = 0.05f;
        float LeftRatio  = 0.25f;
        float RightRatio = 0.60f;
        float DownRatio  = 0.50f;
    };

    class Editor : public Layer
    {
        public:
            Editor();
            Editor(const std::string& p_ProjectPath);
            ~Editor();

            void SetSelectedEntt(Entity p_Entt) { m_SelectedEntt = p_Entt; }
            void UnSelectEntt() { m_SelectedEntt = {}; }

            void OnAttach() override;
            void OnDetach() override;
            void OnUpdate() override;
            void OnRender() override;
            void OnImgui() override;
            void OnEvent(Event& p_Event) override;

            void SaveSceneAs(AssetHandle p_Scene);
            void SaveScene(AssetHandle p_Scene);

            bool IsSelected(Entity p_Entt) const { return p_Entt == m_SelectedEntt; }
            Entity GetSelected() const { return m_SelectedEntt; }
            Ref<AssetImporterPanel> GetAssetImporterPanel() { return m_AssetImporter; }
            Ref<MaterialPanel> GetMaterialPanel() { return m_MaterialPanel; }
            Ref<SceneEditPanel> GetSceneEditPanel() { return m_SceneEditPanel; }
            Ref<TextureAtlasPanel> GetTextureAtlasPanel() { return m_TextureAtlasPanel; }
            Ref<AnimationPanel> GetAnimationPanel() { return m_AnimationPanel; }
            Ref<AnimationControllerPanel> GetAnimationControllerPanel() { return m_AnimationControllerPanel; }

            const Ref<EditorCamera>& GetCamera() { return m_Camera; }

            void SetState(RuntimeState p_State) { m_State = p_State; }
            RuntimeState GetState() const { return m_State; }

            void OpenProject(const std::filesystem::path& p_Path);

            void SetGuizmoType(int p_Type) { m_GuizmoType = p_Type; }
            int GetGuizmoType() const { return m_GuizmoType; }

            static void BeginDockspace(std::string p_ID, std::string p_Dockspace, bool p_MenuBar, ImGuiDockNodeFlags p_DockFlags = 0);
            static void EndDockspace();

        private:
            void Init();

            void DrawMenuBar();
            void Shortcuts();

            void OpenScene();
            void SaveSceneAs();
            void SaveScene();

            void BuildDefaultLayout();

        private:
            std::vector<Ref<EditorPanel>> m_Panels;
            Ref<SettingsPanel> m_Settings                            = nullptr;
            Ref<AssetImporterPanel> m_AssetImporter                  = nullptr;
            Ref<ProjectExporterPanel> m_ProjectExporter              = nullptr;
            Ref<MaterialPanel> m_MaterialPanel                       = nullptr;
            Ref<SceneEditPanel> m_SceneEditPanel                     = nullptr;
            Ref<TextureAtlasPanel> m_TextureAtlasPanel               = nullptr;
            Ref<AnimationPanel> m_AnimationPanel                     = nullptr;
            Ref<AnimationControllerPanel> m_AnimationControllerPanel = nullptr;

            std::unordered_map<EditorPanelDock, ImGuiID> m_DockNodes;
            LayoutConfig m_LayoutConfig                              = {};
            bool m_LayoutInitialized                                 = false;

            Entity m_SelectedEntt                                    = {};
            RuntimeState m_State                                     = RuntimeState::Edit;

            Ref<EditorCamera> m_Camera                               = nullptr;
            bool m_CaptureShortcuts                                  = true;

            std::filesystem::path m_ProjectPath                      = "";
            int m_GuizmoType                                         = 0;

    };

} // namespace KTN