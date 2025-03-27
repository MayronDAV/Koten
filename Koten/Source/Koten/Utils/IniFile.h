#pragma once
#include "Koten/Core/Base.h"

// std
#include <string>
#include <unordered_map>


namespace KTN
{
	class KTN_API IniFile
	{
		public:
			IniFile(const std::string& p_Filepath, const std::string& p_DefaultGroup = "");
			~IniFile() = default;

			void Reload();
			void Rewrite() const;

			template <typename T>
			T Get(const std::string& p_Group, const std::string& p_Key);
			template <typename T>
			T GetOrDefault(const std::string& p_Group, const std::string& p_Key, T p_Default);
			template <typename T>
			bool Set(const std::string& p_Group, const std::string& p_Key, const T& p_Value);
			template <typename T>
			bool Add(const std::string& p_Group, const std::string& p_Key, const T& p_Value);
			template <typename T>
			bool SetOrAdd(const std::string& p_Group, const std::string& p_Key, const T& p_Value);

			bool Remove(const std::string& p_Group, const std::string& p_Key);
			void RemoveAll();
			bool IsKeyExisting(const std::string& p_Group, const std::string& p_Key) const;
			bool IsGroupExisting(const std::string& p_Group) const;

			void RegisterPair(const std::string& p_Key, const std::string& p_Value);
			void RegisterPair(const std::string& p_Group, const std::string& p_Key, const std::string& p_Value);
			void RegisterPair(const std::pair<std::string, std::string>& p_Pair);
			void RegisterPair(const std::string& p_Group, const std::pair<std::string, std::string>& p_Pair);

			void Load();

			static std::pair<std::string, std::string> ExtractKeyAndValue(const std::string& p_Line);
			static bool IsValidLine(const std::string& p_Line);
			static bool StringToBoolean(const std::string& p_Value);

			const std::string& GetFilePath() const { return m_FilePath; }
			const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& GetData() const { return m_Data; }

		private:
			std::string m_DefaultGroup;
			std::string m_FilePath;
			std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_Data;
	};

	template <typename T>
	T IniFile::Get(const std::string& p_Group, const std::string& p_Key)
	{
		if (!IsKeyExisting(p_Group, p_Key))
		{
			if constexpr (std::is_same<bool, T>::value)
				return false;
			else if constexpr (std::is_same<std::string, T>::value)
				return "NULL";
			else if constexpr (std::is_integral<T>::value)
				return static_cast<T>(0);
			else if constexpr (std::is_floating_point<T>::value)
				return static_cast<T>(0.0f);
			else
			{
				KTN_CORE_ASSERT(false, "The given type must be : bool, integral, floating point or string");
				return T();
			}
		}

		const std::string& value = m_Data[p_Group][p_Key];

		if constexpr (std::is_same<bool, T>::value)
			return StringToBoolean(value);
		else if constexpr (std::is_same<std::string, T>::value)
			return value;
		else if constexpr (std::is_integral<T>::value)
			return static_cast<T>(std::atoi(value.c_str()));
		else if constexpr (std::is_floating_point<T>::value)
			return static_cast<T>(std::atof(value.c_str()));
		else
		{
			KTN_CORE_ASSERT(false, "Type not supported");
			return T();
		}
	}

	template <typename T>
	T IniFile::GetOrDefault(const std::string& p_Group, const std::string& p_Key, T p_Default)
	{
		return IsKeyExisting(p_Group, p_Key) ? Get<T>(p_Group, p_Key) : p_Default;
	}

	template <typename T>
	bool IniFile::Set(const std::string& p_Group, const std::string& p_Key, const T& p_Value)
	{
		if (!IsKeyExisting(p_Group, p_Key))
			return false;

		if constexpr (std::is_same<bool, T>::value)
			m_Data[p_Group][p_Key] = p_Value ? "true" : "false";
		else if constexpr (std::is_same<std::string, T>::value)
			m_Data[p_Group][p_Key] = p_Value;
		else if constexpr (std::is_integral<T>::value)
			m_Data[p_Group][p_Key] = std::to_string(p_Value);
		else if constexpr (std::is_floating_point<T>::value)
			m_Data[p_Group][p_Key] = std::to_string(p_Value);
		else
		{
			KTN_CORE_ASSERT(false, "Type not supported");
			return false;
		}

		return true;
	}

	template <typename T>
	bool IniFile::SetOrAdd(const std::string& p_Group, const std::string& p_Key, const T& p_Value)
	{
		if (IsKeyExisting(p_Group, p_Key))
			return Set(p_Group, p_Key, p_Value);
		else
			return Add(p_Group, p_Key, p_Value);
	}

	template <typename T>
	bool IniFile::Add(const std::string& p_Group, const std::string& p_Key, const T& p_Value)
	{
		if (IsKeyExisting(p_Group, p_Key))
			return false;

		if constexpr (std::is_same<bool, T>::value)
			m_Data[p_Group][p_Key] = p_Value ? "true" : "false";
		else if constexpr (std::is_same<std::string, T>::value)
			m_Data[p_Group][p_Key] = p_Value;
		else if constexpr (std::is_integral<T>::value)
			m_Data[p_Group][p_Key] = std::to_string(p_Value);
		else if constexpr (std::is_floating_point<T>::value)
			m_Data[p_Group][p_Key] = std::to_string(p_Value);
		else
		{
			KTN_CORE_ASSERT(false, "Type not supported");
			return false;
		}

		return true;
	}

} // namespace KTN
