#pragma once
#include "Koten/Koten.h"
#include "EditorPanel.h"
#include "EditorCamera.h"


namespace KTN
{
	class SettingsPanel;

	class Editor : public Layer
	{
		public:
			Editor();
			~Editor();

			void SetSelectedEntt(Entity p_Entt) { m_SelectedEntt = p_Entt; }
			void UnSelectEntt() { m_SelectedEntt = {}; }

			void OnAttach() override;
			void OnDetach() override;
			void OnUpdate() override;
			void OnRender() override;
			void OnImgui() override;
			void OnEvent(Event& p_Event) override;

			bool IsSelected(Entity p_Entt) const { return p_Entt == m_SelectedEntt; }
			Entity GetSelected() const { return m_SelectedEntt; }

			const Ref<EditorCamera>& GetCamera() { return m_Camera; }

		private:
			void DrawMenuBar();
			void Shortcuts();

			void OpenScene();
			void SaveSceneAs();

		private:
			Ref<Texture2D> m_MainTexture = nullptr;
			uint32_t m_Width = 800;
			uint32_t m_Height = 600;

			Ref<Scene> m_ActiveScene = nullptr;

			std::vector<Ref<EditorPanel>> m_Panels;
			Ref<SettingsPanel> m_Settings = nullptr;

			Entity m_SelectedEntt;

			Ref<EditorCamera> m_Camera = nullptr;
			bool m_CaptureShortcuts = true;
	};

} // namespace KTN