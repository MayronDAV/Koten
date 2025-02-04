#pragma once


#ifdef KTN_WINDOWS
	#ifdef KTN_EXPORT
		#define KTN_API __declspec(dllexport)
	#else
		#define KTN_API __declspec(dllimport)
	#endif
#elif defined(KTN_LINUX)
	#ifdef KTN_EXPORT
		#define KTN_API __attribute__((visibility("default")))
	#else
		#define KTN_API
	#endif
#else
	#error Currently only support Windows and Linux!
#endif