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
			Camera m_Camera;
			float m_Distance = 5.0f;
			float m_Zoom = 1.0f;
			float m_Speed = 4.0f;
			bool m_Orthographic = false;
			glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
			uint32_t m_Width = 800;
			uint32_t m_Height = 600;

			Ref<Scene> m_ActiveScene = nullptr;
	};

} // namespace KTN