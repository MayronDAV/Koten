#pragma once
#include "Koten/Koten.h"
#include "EditorPanel.h"
#include "EditorCamera.h"


namespace KTN
{
	class SettingsPanel;

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

			void OpenScene(AssetHandle p_Handle);

			void OnAttach() override;
			void OnDetach() override;
			void OnUpdate() override;
			void OnRender() override;
			void OnImgui() override;
			void OnEvent(Event& p_Event) override;

			bool IsSelected(Entity p_Entt) const { return p_Entt == m_SelectedEntt; }
			Entity GetSelected() const { return m_SelectedEntt; }

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

		private:
			Ref<Scene> m_ActiveScene = nullptr;

			std::vector<Ref<EditorPanel>> m_Panels;
			Ref<SettingsPanel> m_Settings = nullptr;

			Entity m_SelectedEntt;
			RuntimeState m_State = RuntimeState::Edit;

			Ref<EditorCamera> m_Camera = nullptr;
			bool m_CaptureShortcuts = true;
	};

} // namespace KTN