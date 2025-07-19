#include "ktnpch.h"
#include "Engine.h"



namespace KTN
{
	void Engine::Init()
	{
		KTN_PROFILE_FUNCTION();

		if (s_Instance)
			Shutdown();

		s_Instance = new Engine();
	}

	void Engine::Shutdown()
	{
		KTN_PROFILE_FUNCTION();

		if (s_Instance)
		{
			delete s_Instance;
			s_Instance = nullptr;
		}
	}

	struct Header
	{
		char Magic[4] = { 'K', 'T', 'B', 'N' };
		int Version = 1;
	};

	bool Engine::SaveSettings(const std::filesystem::path& p_Folder)
	{
		KTN_PROFILE_FUNCTION();

		std::ofstream out( p_Folder / "Engine.ktbin", std::ios::binary | std::ios::trunc );
	
		Header header = {};
		out.write(reinterpret_cast<const char*>(&header), sizeof(header));

		out.write(reinterpret_cast<const char*>(&m_Settings), sizeof(m_Settings));

		return true;
	}

	bool Engine::LoadSettings(const std::filesystem::path& p_Folder)
	{
		KTN_PROFILE_FUNCTION();

		auto path = p_Folder / "Engine.ktbin";
		if (!FileSystem::Exists(path.string()))
			return false;

		std::ifstream in(path, std::ios::binary );
		if (!in)
		{
			KTN_CORE_ERROR("Failed to open engine file");
			return false;
		}

		Header header = {};
		in.read(reinterpret_cast<char*>(&header), sizeof(header));
		if (memcmp(header.Magic, "KTBN", 4) != 0 || header.Version != 1)
		{
			KTN_CORE_ERROR("Invalid engine file!");
			return false;
		}

		in.read(reinterpret_cast<char*>(&m_Settings), sizeof(m_Settings));

		return true;
	}

} // namespace KTN