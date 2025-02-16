#pragma once


#if defined(KTN_PROFILE_ENABLED)
	// lib
	#include <optick.h>

	#define KTN_PROFILE_LOW 1

	#define KTN_PROFILE_FRAME(name) OPTICK_FRAME(name)
	#define KTN_PROFILE_FUNCTION() OPTICK_EVENT()
	#define KTN_PROFILE_SCOPE_T(name, category) OPTICK_CATEGORY(name, category)
	#define KTN_PROFILE_SCOPE(name) KTN_PROFILE_SCOPE_T(name, Optick::Category::Debug)
	#define KTN_PROFILE_SHUTDOWN() OPTICK_SHUTDOWN()

	#if KTN_PROFILE_LOW
		#define KTN_PROFILE_FUNCTION_LOW() OPTICK_EVENT()
		#define KTN_PROFILE_SCOPE_T_LOW(name, category) KTN_PROFILE_SCOPE_T(name, category)
		#define KTN_PROFILE_SCOPE_LOW(name) KTN_PROFILE_SCOPE_T_LOW(name, Optick::Category::Debug)
	#else
		#define KTN_PROFILE_FUNCTION_LOW()
		#define KTN_PROFILE_SCOPE_T_LOW(name, category)
		#define KTN_PROFILE_SCOPE_LOW(name)
	#endif

#else
	#define KTN_PROFILE_FRAME(name)
	#define KTN_PROFILE_FUNCTION()
	#define KTN_PROFILE_SCOPE_T(name, category)
	#define KTN_PROFILE_SCOPE(name)
	#define KTN_PROFILE_SHUTDOWN()
	#define KTN_PROFILE_FUNCTION_LOW()
	#define KTN_PROFILE_SCOPE_T_LOW(name, category)
	#define KTN_PROFILE_SCOPE_LOW(name)
#endif
