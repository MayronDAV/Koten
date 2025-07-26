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

	std::string FileSystem::GetParent(const std::string& p_Path)
	{
		auto path = std::filesystem::path(p_Path).parent_path();
		return path.string();
	}

	std::string FileSystem::GetAbsolute(const std::string& p_Path)
	{
		auto path = std::filesystem::absolute(p_Path);
		return path.string();
	}

	std::string FileSystem::GetRelative(const std::string& p_Path, const std::string& p_BasePath)
	{
		auto path = std::filesystem::relative(p_Path, p_BasePath);
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

	Buffer FileSystem::ReadFileBinary(const std::string& p_Path)
	{
		std::ifstream stream(p_Path, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			KTN_CORE_ERROR("Could not open file: {}", p_Path);
			return {};
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint64_t size = end - stream.tellg();

		if (size == 0)
		{
			// File is empty
			return {};
		}

		Buffer buffer(size);
		stream.read(buffer.As<char>(), size);
		stream.close();
		return buffer;
	}

	void FileSystem::Search(const std::string& p_Query, const std::string& p_Dir, std::vector<std::filesystem::path>& p_Results)
	{
		for (const auto& entry : std::filesystem::recursive_directory_iterator(p_Dir))
		{
			if (entry.is_regular_file() && entry.path().filename().string().find(p_Query) != std::string::npos)
				p_Results.push_back(entry.path());
		}
	}

	bool FileSystem::Exists(const std::string& p_Path)
	{
		return std::filesystem::exists(p_Path);
	}

	void FileSystem::CreateDirectories(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		if (!std::filesystem::exists(p_Path))
			std::filesystem::create_directories(p_Path);
	}

	void FileSystem::Copy(const std::string& p_Src, const std::string& p_Dest)
	{
		KTN_PROFILE_FUNCTION();

		std::filesystem::create_directories(p_Dest);
		std::filesystem::copy(p_Src, p_Dest, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
	}

	void FileSystem::Rename(const std::string& p_Path, const std::string& p_Name)
	{
		KTN_PROFILE_FUNCTION();

		std::filesystem::path path = p_Path;
		std::filesystem::rename(path, path.parent_path() / p_Name);
	}

	void FileSystem::Remove(const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		std::filesystem::remove_all(std::filesystem::absolute(p_Path));
	}

} // namespace KTN
