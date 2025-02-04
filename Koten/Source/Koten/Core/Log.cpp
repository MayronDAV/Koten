#include "ktnpch.h"
#include "Log.h"

// lib
#include <spdlog/sinks/stdout_color_sinks.h>



namespace KTN
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		//spdlog::set_pattern("%^[%T] %n: %v%$");
		spdlog::set_pattern("%n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("CORE");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}

} // namespace KTN