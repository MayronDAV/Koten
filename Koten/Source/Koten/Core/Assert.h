#pragma once

#include "Log.h"

// std
#include <filesystem>



#if defined(KTN_WINDOWS) && defined(_MSC_VER)
	#define KTN_DEBUGBREAK() __debugbreak()
#elif defined(KTN_LINUX) && (defined(__GNUC__) || defined(__clang__))
	#include <csignal>
	#if defined(__i386__) || defined(__x86_64__)
		#define KTN_DEBUGBREAK() __asm__ volatile("int3")
	#elif defined(__aarch64__) || defined(__arm__)
		#define KTN_DEBUGBREAK() __builtin_trap()
	#elif defined(SIGTRAP)
		#define KTN_DEBUGBREAK() raise(SIGTRAP)
	#else
		#define KTN_DEBUGBREAK() raise(SIGABRT)
	#endif
#else
	#error KTN_DEBUGBREAK() currently only support windows (msvc) and linux (gcc/clang)!
#endif


#ifdef KTN_DEBUG
	#define KTN_ENABLE_ASSERTS
#endif

#ifndef KTN_DIST
	#define KTN_ENABLE_VERIFY
#endif

#define KTN_EXPAND_MACRO(x) x
#define KTN_STRINGIFY_MACRO(x) #x


#ifdef KTN_ENABLE_ASSERTS
	#define KTN_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { KTN##type##ERROR(msg, __VA_ARGS__); KTN_DEBUGBREAK(); } }
	#define KTN_INTERNAL_ASSERT_WITH_MSG(type, check, ...) KTN_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define KTN_INTERNAL_ASSERT_NO_MSG(type, check) KTN_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", KTN_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define KTN_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define KTN_INTERNAL_ASSERT_GET_MACRO(...) KTN_EXPAND_MACRO( KTN_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, KTN_INTERNAL_ASSERT_WITH_MSG, KTN_INTERNAL_ASSERT_NO_MSG) )

	#define KTN_ASSERT(...) KTN_EXPAND_MACRO( KTN_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define KTN_CORE_ASSERT(...) KTN_EXPAND_MACRO( KTN_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define KTN_ASSERT(...)
	#define KTN_CORE_ASSERT(...)
#endif

#ifdef KTN_ENABLE_VERIFY
	#define KTN_INTERNAL_VERIFY_IMPL(type, check, msg, ...) { if(!(check)) { KTN##type##ERROR(msg, __VA_ARGS__); KTN_DEBUGBREAK(); } }
	#define KTN_INTERNAL_VERIFY_WITH_MSG(type, check, ...) KTN_INTERNAL_VERIFY_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define KTN_INTERNAL_VERIFY_NO_MSG(type, check) KTN_INTERNAL_VERIFY_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", KTN_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define KTN_INTERNAL_VERIFY_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define KTN_INTERNAL_VERIFY_GET_MACRO(...) KTN_EXPAND_MACRO( KTN_INTERNAL_VERIFY_GET_MACRO_NAME(__VA_ARGS__, KTN_INTERNAL_VERIFY_WITH_MSG, KTN_INTERNAL_VERIFY_NO_MSG) )

	#define KTN_VERIFY(...) KTN_EXPAND_MACRO( KTN_INTERNAL_VERIFY_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define KTN_CORE_VERIFY(...) KTN_EXPAND_MACRO( KTN_INTERNAL_VERIFY_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define KTN_VERIFY(...)
	#define KTN_CORE_VERIFY(...)
#endif