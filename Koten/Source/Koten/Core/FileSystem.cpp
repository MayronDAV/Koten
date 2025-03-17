#include "ktnpch.h"
#include "FileSystem.h"




namespace KTN
{
	std::string FileSystem::GetName(const std::string& p_Path)
	{
		auto path = std::filesystem::path(p_Path).filename();
		return path.string();
	}

	std::string FileSystem::GetStem(const std::string& p_Path)
	{
		auto path = std::filesystem::path(p_Path).stem();
		return path.string();
	}

	std::string FileSystem::GetExtension(const std::string& p_Path)
	{
		auto path = std::filesystem::path(p_Path).extension();
		return path.string();
	}

	bool FileSystem::WriteFile(const std::string& p_Path, uint8_t* p_Buffer, uint32_t p_Size)
	{
		std::ofstream stream(p_Path, std::ios::binary | std::ios::trunc);

		if (!stream)
		{
			stream.close();
			return false;
		}

		stream.write((char*)p_Buffer, p_Size);
		stream.close();

		return true;
	}

	bool FileSystem::WriteTextFile(const std::string& p_Path, const std::string& p_Text)
	{
		return WriteFile(p_Path, (uint8_t*)&p_Text[0], (uint32_t)p_Text.size());
	}

	std::string FileSystem::ReadFile(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		std::string result;
		std::ifstream shaderFile{ p_Path, std::ios::in | std::ios::binary };

		if (!shaderFile)
		{
			KTN_CORE_ERROR("Could not open file: {}", p_Path);
			return std::string();
		}

		shaderFile.seekg(0, std::ios::end);
		result.resize(shaderFile.tellg());
		shaderFile.seekg(0, std::ios::beg);

		shaderFile.read(&result[0], result.size());
		shaderFile.close();

		return result;
	}

	void FileSystem::CreateDirectories(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		if (!std::filesystem::exists(p_Path))
			std::filesystem::create_directories(p_Path);
	}

} // namespace KTN
