#pragma once

#include "Koten/Koten.h"

// std
#include <string>
#include <unordered_map>


namespace KTN
{
	class Shortcuts
	{
		public:
			static void Init();
			static bool IsActionPressed(const std::string& p_Action);

			static void PushShortcutKey(const std::string& p_Action, int p_Key);
			static void SetShortcut(const std::string& p_Action, const std::vector<int>& p_Keys);

			static void RemoveShortcut(const std::string& p_Action);
			static std::vector<int> StringToKeys(const std::string& p_String);
			static std::string KeysToString(const std::vector<int>& p_Keys);

			static std::vector<int> GetShortcut(const std::string& p_Action) { return s_Shortcuts[p_Action]; }
			static std::string GetShortcutStr(const std::string& p_Action) { return KeysToString(s_Shortcuts[p_Action]); }

			static void UploadShortcuts();
			static std::unordered_map<std::string, std::vector<int>>& GetShortcuts() { return s_Shortcuts; }

		private:
			inline static IniFile m_File{ "Resources/Shortcuts.ini" };
			static std::unordered_map<std::string, std::vector<int>> s_Shortcuts;
	};

} // namespace KTN