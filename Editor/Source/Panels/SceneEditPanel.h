#pragma once
#include "EditorPanel.h"
#include "Koten/Scene/Scene.h"


namespace KTN
{
	class SceneEditPanel : public EditorPanel
	{
		public:
			SceneEditPanel();
			~SceneEditPanel() override = default;

			void Create(AssetHandle p_Scene = AssetHandle());
			void Edit(AssetHandle p_Scene);
			void OnImgui() override;

		private:
			std::string m_SceneFolder = "";
			std::string m_SceneName = "NewScene";
			bool m_IsCreating = false;
			bool m_IsEditing = false;
			SceneConfig m_SceneConfig = {};
			AssetHandle m_SceneHandle = 0;
	};

} // namespace KTN
