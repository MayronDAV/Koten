#pragma once
#include "Koten/Koten.h"
#include "EditorPanel.h"



namespace KTN
{
	class HierarchyPanel : public EditorPanel
	{
		public:
			HierarchyPanel();
			~HierarchyPanel() override = default;

			void OnImgui() override;

		private:
			void DrawEnttNode(Entity p_Entt);
	};


} // namespace KTN