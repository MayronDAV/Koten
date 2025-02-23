#pragma once
#include "EditorPanel.h"



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
			Ref<Texture2D> m_MainTexture	= nullptr;
			uint32_t m_Width				= 800;
			uint32_t m_Height				= 600;
	};

} // namespace KTN
