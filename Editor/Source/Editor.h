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
		Play
	};

	class Editor : public Layer
	{
		public:
			Editor();
			~Editor();

			void SetSelectedEntt(Entity p_Entt) { m_SelectedEntt = p_Entt; }
			void UnSelectEntt() { m_SelectedEntt = {}; }

			void OpenScene(const std::string& p_Path);

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

		private:
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