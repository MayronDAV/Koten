#pragma once

#include "Base.h"
#include "Definitions.h"

// std
#include <filesystem>



namespace KTN
{
	struct Statistics
	{
		uint32_t FramesPerSecond	= 0;

		uint32_t TrianglesCount		= 0;
		uint32_t DrawCalls			= 0;
	};

	struct Settings
	{
		bool AutoRecompile = true;
		bool MousePicking = true;

		bool ShowDebugPhysicsCollider = false;
		bool ShowDebugAABB = false;
		bool ShowDebugBVH = false;
		float DebugLineWidth = 2.0f;
		float DebugCircleThickness = 0.03f;

		bool UpdateMinimized = true;

		uint32_t Width = 800;
		uint32_t Height = 600;
		WindowMode Mode = WindowMode::Windowed;
		bool Resizable = true;
		bool Maximize = false;
		bool Center = true;
		bool Vsync = false;
	};

	class KTN_API Engine
	{
		public:
			RenderAPI GetAPI() const { return m_API; }
			void SetAPI(RenderAPI p_API) { m_API = p_API; }

			Statistics& GetStats() { return m_Stats; }
			void ResetStats()
			{
				m_Stats.TrianglesCount  = 0;
				m_Stats.DrawCalls		= 0;
			}

			Settings& GetSettings() { return m_Settings; }

			bool SaveSettings(const std::filesystem::path& p_Folder = "Resources");
			bool LoadSettings(const std::filesystem::path& p_Folder = "Resources");

			static void Init();
			static void Shutdown();
			static Engine& Get() { return *s_Instance; }

		private:
			RenderAPI m_API = RenderAPI::OpenGL;
			Statistics m_Stats = {};
			Settings m_Settings = {};

			inline static Engine* s_Instance = nullptr;
	};

} // namespace KTN
