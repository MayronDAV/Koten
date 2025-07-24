#pragma once
#include "EditorPanel.h"

// std
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>


namespace KTN
{
	struct Tab
	{
		std::string Name;
		std::function<void()> Content;
	};

	class SettingsPanel : public EditorPanel
	{
		public:
			SettingsPanel();
			~SettingsPanel() override = default;

			void AddTab(const std::string& p_TabHeader, const Tab& p_Tab) { m_Tabs[p_TabHeader].push_back(p_Tab); }

			void OnImgui() override;

		private:
			bool DrawNode(const std::string& p_Name, bool p_IsHeader = false);

		private:
			std::unordered_map<std::string, std::vector<Tab>> m_Tabs;
			std::string m_Selected = "";
	};

} // namespace KTN
