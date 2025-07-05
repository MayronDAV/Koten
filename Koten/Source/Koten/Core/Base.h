#pragma once

#include "Koten/Debug/Profile.h"

// std
#include <cstdint>
#include <memory>

#define GLM_ENABLE_EXPERIMENTAL


#ifdef KTN_WINDOWS
	#ifdef KTN_EXPORT
		#define KTN_API __declspec(dllexport)
	#else
		#define KTN_API __declspec(dllimport)
	#endif
	#define YAML_CPP_NO_EXPORT
#elif defined(KTN_LINUX)
	#ifdef KTN_EXPORT
		#define KTN_API __attribute__((visibility("default")))
	#else
		#define KTN_API
	#endif

	#define YAML_CPP_NO_EXPORT __attribute__((visibility("hidden")))
#else
	#error Currently only support Windows and Linux!
#endif

#define YAML_CPP_API KTN_API

#define BIT(x) (1 << x)

namespace KTN
{
	template<typename T>
	using Unique = std::unique_ptr<T>;
	template<typename T, typename ...Args>
	inline constexpr Unique<T> CreateUnique(Args&& ...p_Args)
	{
		return std::make_unique<T>(std::forward<Args>(p_Args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ...Args>
	inline constexpr Ref<T> CreateRef(Args&& ...p_Args)
	{
		return std::make_shared<T>(std::forward<Args>(p_Args)...);
	}

	template<class T, class U>
	inline constexpr Ref<U> As(const Ref<T>& p_Class)
	{
		static_assert(std::is_convertible_v<U*, T*> || std::is_base_of_v<U, T>,
			"Invalid cast: T must be convertible to U or U must be a base of T.");
		return std::dynamic_pointer_cast<U>(p_Class);
	}


} // namespace KTN

#define KTN_GLFWLOG "[ GLFW ] : "
#define KTN_GLLOG	"[OPENGL] : "
#define KTN_VKLOG	"[VULKAN] : "
#define KTN_FONTLOG "[ FONT ] : "

#ifdef KTN_WINDOWS
	#define Strncpy(dst, src, n) strncpy_s(dst, src, n - 1); dst[n - 1] = '\0';
	#define Strcpy(dst, src) strcpy_s(dst, src)
	#define SScanf(buf, fmt, ...) sscanf_s(buf, fmt, __VA_ARGS__)
#else
	#define Strncpy(dst, src, n) strncpy(dst, src, n - 1); dst[n - 1] = '\0';
	#define Strcpy(dst, src) strcpy(dst, src)
	#define SScanf(buf, fmt, ...) sscanf(buf, fmt, __VA_ARGS__)
#endif