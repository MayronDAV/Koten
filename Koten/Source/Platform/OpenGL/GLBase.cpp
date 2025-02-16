#include "ktnpch.h"
#include "GLBase.h"



namespace KTN
{
#if GL_DEBUG
	bool glLogCall(const char* p_Function, const char* p_File, const int32_t p_Line)
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLenum err(glGetError());
		bool Error = true;
		while (err != GL_NO_ERROR)
		{
			std::string error;

			switch (err)
			{
				case GL_INVALID_OPERATION:
					error = "GL_INVALID_OPERATION";
					break;
				case GL_INVALID_ENUM:
					error = "GL_INVALID_ENUM";
					break;
				case GL_INVALID_VALUE:
					error = "GL_INVALID_VALUE";
					break;
				case GL_OUT_OF_MEMORY:
					error = "GL_OUT_OF_MEMORY";
					break;
				case GL_INVALID_FRAMEBUFFER_OPERATION:
					error = "GL_INVALID_FRAMEBUFFER_OPERATION";
					break;
				default:;
			}

			std::cerr << error.c_str() << " - " << p_File << " - " << p_Function << ":" << p_Line << std::endl;
			Error = false;
			err = glGetError();
		}
		return Error;
	}

	void glClearError()
	{
		KTN_PROFILE_FUNCTION_LOW();

		while (glGetError() != GL_NO_ERROR);
	}
#endif

} // namespace LNR