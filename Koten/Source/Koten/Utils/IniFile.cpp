#include "ktnpch.h"
#include "IniFile.h"
#include "Koten/Core/FileSystem.h"
#include "Koten/Utils/StringUtils.h"

// std
#include <filesystem>
#include <fstream>



namespace KTN
{
	IniFile::IniFile(const std::string& p_Filepath, const std::string& p_DefaultGroup)
		: m_FilePath(p_Filepath), m_DefaultGroup(p_DefaultGroup)
	{
		Load();
	}

	void IniFile::Reload()
	{
		RemoveAll();
		Load();
	}

	void IniFile::Rewrite() const 
	{
		if (m_FilePath.empty()) 
		{
			KTN_CORE_WARN("Ini file path empty");
			return;
		}

		std::stringstream stream;

		for (const auto& [group, keyValuePairs] : m_Data) 
		{
			if (group != m_DefaultGroup) 
				stream << "[" << group << "]" << std::endl;

			for (const auto& [key, value] : keyValuePairs) 
			{
				stream << key << "=" << value << std::endl;
			}

			if (group != m_DefaultGroup)
				stream << std::endl;
		}

		FileSystem::WriteTextFile(m_FilePath, stream.str());
	}

	bool IniFile::Remove(const std::string& p_Group, const std::string& p_Key)
	{
		if (IsKeyExisting(p_Group, p_Key))
		{
			m_Data[p_Group].erase(p_Key);
			return true;
		}

		return false;
	}

	void IniFile::RemoveAll()
	{
		m_Data.clear();
	}

	bool IniFile::IsKeyExisting(const std::string& p_Group, const std::string& p_Key) const
	{
		auto groupIt = m_Data.find(p_Group);
		if (groupIt == m_Data.end())
			return false;

		return groupIt->second.find(p_Key) != groupIt->second.end();
	}

	bool IniFile::IsGroupExisting(const std::string& p_Group) const
	{
		return m_Data.find(p_Group) != m_Data.end();
	}

	void IniFile::RegisterPair(const std::string& p_Key, const std::string& p_Value)
	{
		RegisterPair(std::make_pair(p_Key, p_Value));
	}

	void IniFile::RegisterPair(const std::string& p_Group, const std::string& p_Key, const std::string& p_Value)
	{
		RegisterPair(p_Group, std::make_pair(p_Key, p_Value));
	}

	void IniFile::RegisterPair(const std::pair<std::string, std::string>& p_Pair)
	{
		m_Data[m_DefaultGroup].insert(p_Pair);
	}

	void IniFile::RegisterPair(const std::string& p_Group, const std::pair<std::string, std::string>& p_Pair)
	{
		m_Data[p_Group].insert(p_Pair);
	}

	void IniFile::Load() 
	{
		if (m_FilePath.empty())
			return;

		if (!FileSystem::Exists(m_FilePath))
			return;

		auto fileString = FileSystem::ReadFile(m_FilePath);
		auto lines = StringUtils::GetLines(fileString);

		std::string currentGroup = m_DefaultGroup;

		for (auto& line : lines)
		{
			line = StringUtils::Trim(line);

			if (line.empty() || line[0] == '#' || line[0] == ';') 
				continue;
			else if (line[0] == '[' && line.back() == ']')
				currentGroup = line.substr(1, line.size() - 2);
			else if (IsValidLine(line)) 
				RegisterPair(currentGroup, ExtractKeyAndValue(line));
		}
	}

	std::pair<std::string, std::string> IniFile::ExtractKeyAndValue(const std::string& p_Line)
	{
		std::string key;
		std::string value;

		std::string* currentBuffer = &key;

		for (auto& c : p_Line)
		{
			if (c == '=')
				currentBuffer = &value;
			else
				currentBuffer->push_back(c);
		}

		return std::make_pair(key, value);
	}

	bool IniFile::IsValidLine(const std::string& p_Line)
	{
		if (p_Line.size() == 0 || p_Line.empty())
			return false;

		if (p_Line[0] == '#' || p_Line[0] == ';' || p_Line[0] == '[')
			return false;

		if (std::count(p_Line.begin(), p_Line.end(), '=') != 1)
			return false;

		return true;
	}

	bool IniFile::StringToBoolean(const std::string& p_Value)
	{
		return (p_Value == "1" || p_Value == "T" || p_Value == "t" || p_Value == "True" || p_Value == "true" || p_Value == "TRUE");
	}
} // namespace KTN