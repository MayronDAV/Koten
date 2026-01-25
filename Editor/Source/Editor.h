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

	enum class RuntimeState
	{
		Edit,
		Play,
		Simulate
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

			const Ref<EditorCamera>& GetCamera() { return m_Camera; }

			void SetState(RuntimeState p_State) { m_State = p_State; }
			RuntimeState GetState() const { return m_State; }

			void OpenProject(const std::filesystem::path& p_Path);

			static void BeginDockspace(std::string p_ID, std::string p_Dockspace, bool p_MenuBar, ImGuiDockNodeFlags p_DockFlags = 0);
			static void EndDockspace();

		private:
			void Init();

			void DrawMenuBar();
			void Shortcuts();

			void OpenScene();
			void SaveSceneAs();
			void SaveScene();

		private:
			std::vector<Ref<EditorPanel>> m_Panels;
			Ref<SettingsPanel> m_Settings = nullptr;
			Ref<AssetImporterPanel> m_AssetImporter = nullptr;
			Ref<ProjectExporterPanel> m_ProjectExporter = nullptr;
			Ref<MaterialPanel> m_MaterialPanel = nullptr;
			Ref<SceneEditPanel> m_SceneEditPanel = nullptr;

			Entity m_SelectedEntt;
			RuntimeState m_State = RuntimeState::Edit;

			Ref<EditorCamera> m_Camera = nullptr;
			bool m_CaptureShortcuts = true;

			std::filesystem::path m_ProjectPath = "";
	};

} // namespace KTN