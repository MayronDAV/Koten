#pragma once

#include "Base.h"
#include "Definitions.h"


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
		float DebugLineWidth = 2.0f;
		float DebugCircleThickness = 0.03f;
	};

	class KTN_API Engine
	{
		public:
			static RenderAPI GetAPI() { return s_API; }
			static void SetAPI(RenderAPI p_API) { s_API = p_API; }

			static Statistics& GetStats() { return s_Stats; }
			static void ResetStats()
			{
				s_Stats.TrianglesCount  = 0;
				s_Stats.DrawCalls		= 0;
			}

			static Settings& GetSettings() { return s_Settings; }

		private:
			static RenderAPI s_API;
			static Statistics s_Stats;
			static Settings s_Settings;
	};

} // namespace KTN
