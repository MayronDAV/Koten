#pragma once
#include "EditorPanel.h"

// lib
#include <imgui.h>



namespace KTN
{
	class SceneViewPanel : public EditorPanel
	{
		public:
			SceneViewPanel();
			~SceneViewPanel() override = default;

			void OnImgui() override;
			void OnUpdate() override;
			void OnRender() override;

		private:
			void ToolWidget();

		private:
			Ref<Texture2D> m_MainTexture	= nullptr;
			uint32_t m_Width				= 800;
			uint32_t m_Height				= 600;

			ImVec2 m_ViewportMinRegion;
			ImVec2 m_ViewportMaxRegion;
			ImVec2 m_ViewportOffset;
			float m_TitlebarHeight = 0.0f;

			int m_GuizmoType = 0;
			bool m_HandleCameraEvents = false;
	};

} // namespace KTN
