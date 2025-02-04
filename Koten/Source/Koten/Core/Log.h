#pragma once

// lib
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>



namespace KTN
{
	class KTN_API Log
	{
		public:
			static void Init();

			inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
			inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

		private:
			static std::shared_ptr<spdlog::logger> s_CoreLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

} // namespace KTN


#ifndef KTN_DIST
	// Core log macros
	#define KTN_CORE_TRACE(...)		::KTN::Log::GetCoreLogger()->trace(__VA_ARGS__);
	#define KTN_CORE_INFO(...)		::KTN::Log::GetCoreLogger()->info(__VA_ARGS__);
	#define KTN_CORE_WARN(...)		::KTN::Log::GetCoreLogger()->warn(__VA_ARGS__);
	#define KTN_CORE_ERROR(...)		::KTN::Log::GetCoreLogger()->error(__VA_ARGS__);
	#define KTN_CORE_CRITICAL(...)	::KTN::Log::GetCoreLogger()->critical(__VA_ARGS__);

	// Client log macros
	#define KTN_TRACE(...)			::KTN::Log::GetClientLogger()->trace(__VA_ARGS__);
	#define KTN_INFO(...)			::KTN::Log::GetClientLogger()->info(__VA_ARGS__);
	#define KTN_WARN(...)			::KTN::Log::GetClientLogger()->warn(__VA_ARGS__);
	#define KTN_ERROR(...)			::KTN::Log::GetClientLogger()->error(__VA_ARGS__);
	#define KTN_CRITICAL(...)		::KTN::Log::GetClientLogger()->critical(__VA_ARGS__);
#else
	// Core log macros
	#define KTN_CORE_TRACE(...)
	#define KTN_CORE_INFO(...)
	#define KTN_CORE_WARN(...)
	#define KTN_CORE_ERROR(...)
	#define KTN_CORE_CRITICAL(...)

	// Client log macros
	#define KTN_TRACE(...)
	#define KTN_INFO(...)
	#define KTN_WARN(...)
	#define KTN_ERROR(...)
	#define KTN_CRITICAL(...)
#endif

