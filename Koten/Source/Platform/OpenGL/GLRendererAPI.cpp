#include "ktnpch.h"
#include "GLBase.h"
#include "GLRendererAPI.h"



namespace KTN
{
	GLRendererAPI::GLRendererAPI()
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_CommandBuffer = CommandBuffer::Create();
		m_CommandBuffer->Init();

		// MaxSamples
		glGetIntegerv(GL_MAX_SAMPLES, &m_Capabilities.MaxSamples);

		// SamplerAnisotropy
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_Capabilities.MaxAnisotropy);
		m_Capabilities.SamplerAnisotropy = (m_Capabilities.MaxAnisotropy > 1.0f);

		// MaxTextureUnits
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_Capabilities.MaxTextureUnits);

		// WideLines
		GLfloat lineWidthRange[2];
		glGetFloatv(GL_LINE_WIDTH_RANGE, lineWidthRange);
		m_Capabilities.WideLines = (lineWidthRange[1] > 1.0f);
		m_Capabilities.MaxLineWidth = lineWidthRange[1];

		m_Capabilities.SupportCompute = GLAD_GL_ARB_compute_shader == 1;

		m_Capabilities.SupportGeometry = GLAD_GL_ARB_geometry_shader4 == 1;

		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glEnable(GL_STENCIL_TEST));
		GLCall(glEnable(GL_CULL_FACE));
		GLCall(glDepthFunc(GL_LEQUAL));
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GLCall(glBlendEquation(GL_FUNC_ADD));
	}

	void GLRendererAPI::ClearColor(const glm::vec4& p_Color)
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glClearColor(p_Color.r, p_Color.g, p_Color.b, p_Color.a));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
	}

	void GLRendererAPI::Begin()
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
		GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	void GLRendererAPI::End()
	{
	}

} // namespace KTN