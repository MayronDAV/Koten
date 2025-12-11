#pragma once
#include "Koten/Koten.h"
#include "EditorPanel.h"



namespace KTN
{
	class MaterialPanel : public EditorPanel
	{
		public:
			MaterialPanel();
			~MaterialPanel() override = default;

			void Open(AssetHandle p_Material = 0);

			void OnImgui() override;

		private:
			Ref<Asset> m_Material = nullptr;
			AssetMetadata m_Metadata = {};
			bool m_Edited = false;
			bool m_New = false;
			AssetHandle m_MaterialHandle = 0;
			bool m_Changed = false;
	};


} // namespace KTN