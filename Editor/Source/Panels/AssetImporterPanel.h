#pragma once
#include "Koten/Asset/Asset.h"
#include "EditorPanel.h"



namespace KTN
{
	class AssetImporterPanel : public EditorPanel
	{
		public:
			AssetImporterPanel() : EditorPanel("AssetImporter") { m_Active = false; }
			~AssetImporterPanel() override;

			void Open() { m_Path = ""; m_Type = AssetType::None; m_Active = true; m_First = true; m_Imported = false; }
			void Open(const std::string& p_Path, AssetType p_Type) { m_Active = true; m_Type = p_Type; m_Path = p_Path; m_First = true; m_Imported = false; }
			void OnImgui() override;

			bool IsImported() const { return m_Imported; }
			bool IsPathImporting(const std::string& p_Path) const { return m_Path == p_Path; }

		private:
			AssetType m_Type = AssetType::None;
			std::string m_Path = "";
			AssetMetadata m_Metadata = {};
			bool m_First = true;
			bool m_Imported = false;
	};

} // namespace KTN
