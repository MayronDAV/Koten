#pragma once
#include "EditorPanel.h"

namespace KTN
{
	class AssetRegistryPanel : public EditorPanel
	{
		public:
			AssetRegistryPanel();
			~AssetRegistryPanel() override = default;

			void OnImgui() override;
	};

} // namespace KTN
