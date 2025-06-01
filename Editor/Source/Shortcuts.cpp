#include "Shortcuts.h"
#include "Koten/OS/Input.h"
#include "Koten/OS/KeyCodes.h"
#include "Koten/Utils/StringUtils.h"

// std
#include <algorithm>
#include <cctype>



namespace KTN
{
	namespace
	{
		int StringToKey(const std::string& p_String)
		{
			// TODO: Add more keys if needed

			std::string str = p_String;
			std::transform(str.begin(), str.end(), str.begin(), ::tolower);

			std::string str1 = p_String;
			std::transform(str1.begin(), str1.end(), str1.begin(), ::toupper);

			#define STRING_TO_KEY(key) if (str1 == #key ) return Key::key;
			#define STRING_TO_KEY2(key, ret) if (str == key ) return ret;

			STRING_TO_KEY2("lsuper", Key::LSuper);
			STRING_TO_KEY2("rsuper", Key::RSuper);
			STRING_TO_KEY2("lctrl", Key::LCtrl);
			STRING_TO_KEY2("rctrl", Key::RCtrl);
			STRING_TO_KEY2("lshift", Key::LShift);
			STRING_TO_KEY2("rshift", Key::RShift);
			STRING_TO_KEY2("lalt", Key::LAlt);
			STRING_TO_KEY2("ralt", Key::RAlt);
			STRING_TO_KEY2("space", Key::Space);
			STRING_TO_KEY2("enter", Key::Enter);
			STRING_TO_KEY2("tab", Key::Tab);
			STRING_TO_KEY2("backspace", Key::Backspace);
			STRING_TO_KEY2("esc", Key::Esc);
			STRING_TO_KEY2("up", Key::Up);
			STRING_TO_KEY2("down", Key::Down);
			STRING_TO_KEY2("left", Key::Left);
			STRING_TO_KEY2("right", Key::Right);
			STRING_TO_KEY2("'", Key::Apostrophe);
			STRING_TO_KEY2(",", Key::Comma);
			STRING_TO_KEY2("-", Key::Minus);
			STRING_TO_KEY2(".", Key::Period);
			STRING_TO_KEY2("/", Key::Slash);
			STRING_TO_KEY2("\\", Key::Backslash);
			STRING_TO_KEY2(";", Key::Semicolon);
			STRING_TO_KEY2("=", Key::Equal);
			STRING_TO_KEY2("[", Key::LeftBracket);
			STRING_TO_KEY2("]", Key::RightBracket);
			STRING_TO_KEY2("`", Key::GraveAccent);
			STRING_TO_KEY2("kp*", Key::KPMultiply);
			STRING_TO_KEY2("kp+", Key::KPAdd);
			STRING_TO_KEY2("kp-", Key::KPSubtract);
			STRING_TO_KEY2("kp.", Key::KPDecimal);
			STRING_TO_KEY2("kp/", Key::KPDivide);
			STRING_TO_KEY2("pause", Key::Pause);

			STRING_TO_KEY(D0);
			STRING_TO_KEY(D1);
			STRING_TO_KEY(D2);
			STRING_TO_KEY(D3);
			STRING_TO_KEY(D4);
			STRING_TO_KEY(D5);
			STRING_TO_KEY(D6);
			STRING_TO_KEY(D7);
			STRING_TO_KEY(D8);
			STRING_TO_KEY(D9);
			STRING_TO_KEY(KP0);
			STRING_TO_KEY(KP1);
			STRING_TO_KEY(KP2);
			STRING_TO_KEY(KP3);
			STRING_TO_KEY(KP4);
			STRING_TO_KEY(KP5);
			STRING_TO_KEY(KP6);
			STRING_TO_KEY(KP7);
			STRING_TO_KEY(KP8);
			STRING_TO_KEY(KP9);
			STRING_TO_KEY(A);
			STRING_TO_KEY(B);
			STRING_TO_KEY(C);
			STRING_TO_KEY(D);
			STRING_TO_KEY(E);
			STRING_TO_KEY(F);
			STRING_TO_KEY(G);
			STRING_TO_KEY(H);
			STRING_TO_KEY(I);
			STRING_TO_KEY(O);
			STRING_TO_KEY(J);
			STRING_TO_KEY(K);
			STRING_TO_KEY(L);
			STRING_TO_KEY(M);
			STRING_TO_KEY(N);
			STRING_TO_KEY(O);
			STRING_TO_KEY(P);
			STRING_TO_KEY(Q);
			STRING_TO_KEY(R);
			STRING_TO_KEY(S);
			STRING_TO_KEY(T);
			STRING_TO_KEY(U);
			STRING_TO_KEY(V);
			STRING_TO_KEY(W);
			STRING_TO_KEY(X);
			STRING_TO_KEY(Y);
			STRING_TO_KEY(Z);
			STRING_TO_KEY(F1);
			STRING_TO_KEY(F2);
			STRING_TO_KEY(F3);
			STRING_TO_KEY(F4);
			STRING_TO_KEY(F5);
			STRING_TO_KEY(F6);
			STRING_TO_KEY(F7);
			STRING_TO_KEY(F8);
			STRING_TO_KEY(F9);
			STRING_TO_KEY(F10);
			STRING_TO_KEY(F11);
			STRING_TO_KEY(F12);

			#undef STRING_TO_KEY
			#undef STRING_TO_KEY2

			return 0;
		}

		std::string KeyToString(int p_Key)
		{
			// TODO: Add more keys if needed

			#define KEY_TO_STRING(key) if (p_Key == Key::key ) return #key;
			#define KEY_TO_STRING2(key, ret) if (p_Key == key ) return ret;

			KEY_TO_STRING2(Key::Apostrophe, "'");
			KEY_TO_STRING2(Key::Comma, ",");
			KEY_TO_STRING2(Key::Minus, "-");
			KEY_TO_STRING2(Key::Period, ".");
			KEY_TO_STRING2(Key::Slash, "/");
			KEY_TO_STRING2(Key::Backslash, "\\");
			KEY_TO_STRING2(Key::Semicolon, ";");
			KEY_TO_STRING2(Key::Equal, "=");
			KEY_TO_STRING2(Key::LeftBracket, "[");
			KEY_TO_STRING2(Key::RightBracket, "]");
			KEY_TO_STRING2(Key::GraveAccent, "`");
			KEY_TO_STRING2(Key::KPMultiply, "kp*");
			KEY_TO_STRING2(Key::KPAdd, "kp+");
			KEY_TO_STRING2(Key::KPSubtract, "kp-");
			KEY_TO_STRING2(Key::KPDecimal, "kp.");
			KEY_TO_STRING2(Key::KPDivide, "kp/");
			KEY_TO_STRING2(Key::Pause, "pause");


			KEY_TO_STRING(Up);
			KEY_TO_STRING(Down);
			KEY_TO_STRING(Left);
			KEY_TO_STRING(Right);
			KEY_TO_STRING(Space);
			KEY_TO_STRING(Enter);
			KEY_TO_STRING(Tab);
			KEY_TO_STRING(Backspace);
			KEY_TO_STRING(Esc);
			KEY_TO_STRING(LSuper);
			KEY_TO_STRING(RSuper);
			KEY_TO_STRING(LCtrl);
			KEY_TO_STRING(RCtrl);
			KEY_TO_STRING(LShift);
			KEY_TO_STRING(RShift);
			KEY_TO_STRING(D0);
			KEY_TO_STRING(D1);
			KEY_TO_STRING(D2);
			KEY_TO_STRING(D3);
			KEY_TO_STRING(D4);
			KEY_TO_STRING(D5);
			KEY_TO_STRING(D6);
			KEY_TO_STRING(D7);
			KEY_TO_STRING(D8);
			KEY_TO_STRING(D9);
			KEY_TO_STRING(KP0);
			KEY_TO_STRING(KP1);
			KEY_TO_STRING(KP2);
			KEY_TO_STRING(KP3);
			KEY_TO_STRING(KP4);
			KEY_TO_STRING(KP5);
			KEY_TO_STRING(KP6);
			KEY_TO_STRING(KP7);
			KEY_TO_STRING(KP8);
			KEY_TO_STRING(KP9);
			KEY_TO_STRING(A);
			KEY_TO_STRING(B);
			KEY_TO_STRING(C);
			KEY_TO_STRING(D);
			KEY_TO_STRING(E);
			KEY_TO_STRING(F);
			KEY_TO_STRING(G);
			KEY_TO_STRING(H);
			KEY_TO_STRING(I);
			KEY_TO_STRING(O);
			KEY_TO_STRING(J);
			KEY_TO_STRING(K);
			KEY_TO_STRING(L);
			KEY_TO_STRING(M);
			KEY_TO_STRING(N);
			KEY_TO_STRING(O);
			KEY_TO_STRING(P);
			KEY_TO_STRING(Q);
			KEY_TO_STRING(R);
			KEY_TO_STRING(S);
			KEY_TO_STRING(T);
			KEY_TO_STRING(U);
			KEY_TO_STRING(V);
			KEY_TO_STRING(W);
			KEY_TO_STRING(X);
			KEY_TO_STRING(Y);
			KEY_TO_STRING(Z);
			KEY_TO_STRING(F1);
			KEY_TO_STRING(F2);
			KEY_TO_STRING(F3);
			KEY_TO_STRING(F4);
			KEY_TO_STRING(F5);
			KEY_TO_STRING(F6);
			KEY_TO_STRING(F7);
			KEY_TO_STRING(F8);
			KEY_TO_STRING(F9);
			KEY_TO_STRING(F10);
			KEY_TO_STRING(F11);
			KEY_TO_STRING(F12);

			#undef KEY_TO_STRING
			#undef KEY_TO_STRING2

			return "";
		}

	} // namespace

	std::unordered_map<std::string, std::vector<int>> Shortcuts::s_Shortcuts;

	void Shortcuts::Init(IniFile& p_File)
	{

		{
			p_File.Add<std::string>("Shortcuts", "Open Scene", "LCtrl+O");
			p_File.Add<std::string>("Shortcuts", "Save Scene As", "LCtrl+LShift+S");
			p_File.Add<std::string>("Shortcuts", "Open Settings", "LCtrl+.");
			p_File.Add<std::string>("Shortcuts", "Reload Scripts", "LCtrl+R");
			p_File.Add<std::string>("Shortcuts", "Play", "LCtrl+P");
			p_File.Add<std::string>("Shortcuts", "Stop", "Pause");
			p_File.Add<std::string>("Shortcuts", "Guizmo None", "Q");
			p_File.Add<std::string>("Shortcuts", "Guizmo Translate", "T");
			p_File.Add<std::string>("Shortcuts", "Guizmo Rotate", "R");
			p_File.Add<std::string>("Shortcuts", "Guizmo Scale", "E");
			p_File.Add<std::string>("Shortcuts", "Guizmo Universal", "U");
			p_File.Rewrite();
		}

		const auto& data = p_File.GetData();
		auto it = data.find("Shortcuts");
		for (const auto& [key, value] : it->second)
		{
			SetShortcut(key, StringToKeys(value));
		}
	}

	bool Shortcuts::IsActionPressed(const std::string& p_Action)
	{
		auto it = s_Shortcuts.find(p_Action);
		if (it != s_Shortcuts.end())
		{
			bool pressed = true;
			for (const auto& key : it->second)
			{
				if (!Input::IsKeyPressed(key))
					pressed = false;
			}

			return pressed;
		}

		return false;
	}

	void Shortcuts::PushShortcutKey(const std::string& p_Action, int p_Key)
	{
		s_Shortcuts[p_Action].push_back(p_Key);
	}

	void Shortcuts::SetShortcut(const std::string& p_Action, const std::vector<int>& p_Keys)
	{
		s_Shortcuts[p_Action] = p_Keys;
	}

	void Shortcuts::RemoveShortcut(const std::string& p_Action)
	{
		s_Shortcuts.erase(p_Action);
	}

	std::vector<int> Shortcuts::StringToKeys(const std::string& p_String)
	{
		std::vector<std::string> strs = StringUtils::Split(p_String, "+");

		std::vector<int> keys;

		for (const auto& str : strs)
		{
			keys.push_back(StringToKey(str));
		}

		return keys;
	}

	std::string Shortcuts::KeysToString(const std::vector<int>& p_Keys)
	{
		std::string keys;

		for (const auto& key : p_Keys)
		{
			std::string str = KeyToString(key);
			if (!str.empty())
				keys += str + "+";
		}

		if (!keys.empty())
			keys.pop_back();

		return keys;
	}

	void Shortcuts::UploadShortcuts(IniFile& p_File)
	{
		for (const auto& [key, value] : s_Shortcuts)
		{
			p_File.Set("Shortcuts", key, KeysToString(value));
		}
		p_File.Rewrite();
	}


} // namespace KTN
