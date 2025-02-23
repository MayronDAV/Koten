#pragma once
#include "EditorPanel.h"



namespace KTN
{
	class InspectorPanel : public EditorPanel
	{
		public:
			InspectorPanel();
			~InspectorPanel() override = default;

			void OnImgui() override;
	};

} // namespace KTN
