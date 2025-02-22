#pragma once
#include "Koten/Koten.h"


namespace KTN
{
	class Editor : public Layer
	{
		public:
			Editor();
			~Editor();

			void OnAttach() override;
			void OnDetach() override;
			void OnUpdate() override;
			void OnRender() override;
			void OnImgui() override;
			void OnEvent(Event& p_Event) override;

		private:
			Ref<Texture2D> m_MainTexture = nullptr;
			Ref<Texture2D> m_CheckerTexture = nullptr;
			Ref<Texture2D> m_WallTexture = nullptr;
			Camera m_Camera;
			float m_Distance = 5.0f;
			float m_Zoom = 1.0f;
			float m_Speed = 4.0f;
			bool m_Orthographic = false;
			int m_MapSize = 5;
			glm::vec2 m_TileSize = { 0.90f, 0.90f };
			glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
			uint32_t m_Width = 800;
			uint32_t m_Height = 600;
	};

} // namespace KTN