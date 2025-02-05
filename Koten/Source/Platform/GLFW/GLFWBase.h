#pragma once
#include "Koten/Core/Base.h"

// lib
#include <GLFW/glfw3.h>


#ifndef KTN_DISABLE_GLFW_LOG
	#define KTN_GLFW_TRACE(...)		KTN_CORE_TRACE(KTN_GLFWLOG __VA_ARGS__)
	#define KTN_GLFW_INFO(...)		KTN_CORE_INFO(KTN_GLFWLOG __VA_ARGS__)
	#define KTN_GLFW_WARN(...)		KTN_CORE_WARN(KTN_GLFWLOG __VA_ARGS__)
	#define KTN_GLFW_ERROR(...)		KTN_CORE_ERROR(KTN_GLFWLOG __VA_ARGS__)
	#define KTN_GLFW_CRITICAL(...)	KTN_CORE_CRITICAL(KTN_GLFWLOG __VA_ARGS__)
#else
	#define KTN_GLFW_TRACE(...)
	#define KTN_GLFW_INFO(...)
	#define KTN_GLFW_WARN(...)
	#define KTN_GLFW_ERROR(...)
	#define KTN_GLFW_CRITICAL(...)
#endif