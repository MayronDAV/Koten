#include "ktnpch.h"
#include "GLContext.h"
#include "GLBase.h"

// lib
#include <GLFW/glfw3.h>



namespace KTN
{
	namespace
	{
	#ifdef GL_DEBUG_CALLBACK
		static std::string GetStringForType(GLenum p_Type)
		{
			switch (p_Type)
			{
				case GL_DEBUG_TYPE_ERROR:				return "Error";
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	return "Deprecated behavior";
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	return "Undefined behavior";
				case GL_DEBUG_TYPE_PORTABILITY:			return "Portability issue";
				case GL_DEBUG_TYPE_PERFORMANCE:			return "Performance issue";
				case GL_DEBUG_TYPE_MARKER:				return "Stream annotation";
				case GL_DEBUG_TYPE_OTHER_ARB:			return "Other";
				default:								return "Unknown";
			}
		}

		static std::string GetStringForSource(GLenum p_Source)
		{
			switch (p_Source)
			{
				case GL_DEBUG_SOURCE_API:				return "API";
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		return "Window System";
				case GL_DEBUG_SOURCE_SHADER_COMPILER:	return "Shader compiler";
				case GL_DEBUG_SOURCE_THIRD_PARTY:		return "Third party";
				case GL_DEBUG_SOURCE_APPLICATION:		return "Application";
				case GL_DEBUG_SOURCE_OTHER:				return "Other";
				default:								return "Unknown";
			}
		}

		static std::string GetStringForSeverity(GLenum p_Severity)
		{
			switch (p_Severity)
			{
				case GL_DEBUG_SEVERITY_HIGH:			return "High";
				case GL_DEBUG_SEVERITY_MEDIUM:			return "Medium";
				case GL_DEBUG_SEVERITY_LOW:				return "Low";
				case GL_DEBUG_SEVERITY_NOTIFICATION:	return "Notification";
				case GL_DEBUG_SOURCE_API:				return "Source API";
				default:								return "Unknown";
			}
		}

		#define KTN_GLDEBUG_LOG(Type)													\
			KTN_GL##Type("Debug Callback:")												\
			KTN_GL##Type("  Message: {}", p_Message)									\
			KTN_GL##Type("  Type: {}", GetStringForType(p_Type))						\
			KTN_GL##Type("  Source: {}", GetStringForSource(p_Source))					\
			KTN_GL##Type("  ID: {}", p_ID)												\
			KTN_GL##Type("  Severity: {}", GetStringForSeverity(p_Severity))			\

		void APIENTRY GLCallbackFunction(GLenum p_Source,
			GLenum p_Type,
			GLuint p_ID,
			GLenum p_Severity,
			GLsizei p_Length,
			const GLchar* p_Message,
			const void* p_UserParam)
		{
			switch (p_Severity)
			{
				case GL_DEBUG_SEVERITY_HIGH:			return KTN_GLDEBUG_LOG(ERROR);
				case GL_DEBUG_SEVERITY_MEDIUM:			return KTN_GLDEBUG_LOG(WARN);
				case GL_DEBUG_SEVERITY_LOW:				return KTN_GLDEBUG_LOG(TRACE);
				case GL_DEBUG_SEVERITY_NOTIFICATION:	return KTN_GLDEBUG_LOG(INFO);
				case GL_DEBUG_SOURCE_API:				return KTN_GLDEBUG_LOG(INFO);
			}
		}
	#endif
	} // namespace

	GLContext::GLContext()
	{
	#if defined(KTN_DEBUG)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	#endif // KTN_DEBUG

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	#if defined(__APPLE__)
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif // __APPLE__
	}

	void GLContext::Init(void* p_Window, const char* p_Name)
	{
		KTN_CORE_ASSERT(p_Window, "Window is nullptr!");

		m_Window = p_Window;
		glfwMakeContextCurrent((GLFWwindow*)m_Window);

		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		KTN_CORE_ASSERT(status, "Failed to initialize glad!");

		KTN_CORE_INFO("----------------------------------");
		KTN_CORE_INFO(KTN_GLLOG);
		KTN_CORE_INFO((const char*)glGetString(GL_VERSION));
		KTN_CORE_INFO((const char*)glGetString(GL_VENDOR));
		KTN_CORE_INFO((const char*)glGetString(GL_RENDERER));
		KTN_CORE_INFO("----------------------------------");

	#if defined(KTN_DEBUG) && defined(GL_DEBUG_CALLBACK)
		KTN_GLINFO("Registering OpenGL debug callback");

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLCallbackFunction, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(
			GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DONT_CARE,
			0,
			&unusedIds,
			GL_TRUE);
	#endif
	}

	void GLContext::SwapBuffer()
	{
		glfwSwapBuffers((GLFWwindow*)m_Window);
	}

	void GLContext::SetVsync(bool p_Value)
	{
		glfwSwapInterval(p_Value ? 1 : 0);
	}

} // namespace KTN
