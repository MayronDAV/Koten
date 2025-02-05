#pragma once
#include "Koten/Core/Base.h"

// lib
#include <glad/glad.h>



#ifndef KTN_DISABLE_GL_LOG
	#define KTN_GLTRACE(...)		KTN_CORE_TRACE(KTN_GLLOG __VA_ARGS__)
	#define KTN_GLINFO(...)			KTN_CORE_INFO(KTN_GLLOG __VA_ARGS__)
	#define KTN_GLWARN(...)			KTN_CORE_WARN(KTN_GLLOG __VA_ARGS__)
	#define KTN_GLERROR(...)		KTN_CORE_ERROR(KTN_GLLOG __VA_ARGS__)
	#define KTN_GLCRITICAL(...)		KTN_CORE_CRITICAL(KTN_GLLOG __VA_ARGS__)
#else
	#define KTN_GLTRACE(...)
	#define KTN_GLINFO(...)
	#define KTN_GLWARN(...)
	#define KTN_GLERROR(...)
	#define KTN_GLCRITICAL(...)
#endif

#ifdef KTN_DEBUG
	#ifdef glDebugMessageCallback
		#define GL_DEBUG_CALLBACK 1
	#else
		#define GL_DEBUG 1
	#endif
#else
	#define GL_DEBUG 0
#endif

namespace KTN
{
#if GL_DEBUG
	bool GLLogCall(const char* p_Function, const char* p_File, const int32_t p_Line);
	void GLClearError();
#endif

#if GL_DEBUG
	#define GLCall(x)                          \
			glClearError();                        \
			x;                                     \
			if(!glLogCall(#x, __FILE__, __LINE__)) \
				LNR_DEBUGBREAK();
#else
	#define GLCall(x) x
#endif

} // namespace KTN