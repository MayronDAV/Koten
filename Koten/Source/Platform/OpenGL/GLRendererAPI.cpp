#include "ktnpch.h"
#include "GLBase.h"
#include "GLUtils.h"
#include "GLRendererAPI.h"
#include "GLTexture.h"



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

	void GLRendererAPI::ClearRenderTarget(const Ref<Texture2D>& p_Texture, uint32_t p_Value)
	{
		KTN_PROFILE_FUNCTION_LOW();

		KTN_CORE_VERIFY(p_Texture != nullptr);

		auto id = As<Texture2D, GLTexture2D>(p_Texture)->GetID();

		if (p_Texture->IsColorAttachment())
		{
			glm::vec4 color = { p_Value, p_Value, p_Value, 1.0f };
			glClearTexImage(id, 0, GLUtils::TextureFormatToGLFormat(p_Texture->GetSpecification().Format), GL_FLOAT, &color[0]);
		}
		else if (p_Texture->IsDepthStencilAttachment())
		{
			float value = 1.0f;
			glClearTexImage(id, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &value);

			if (p_Texture->IsStencil())
			{
				GLint clearStencil = 0;
				glClearTexImage(id, 0, GL_STENCIL_INDEX, GL_INT, &clearStencil);
			}
		}
	}

	void GLRendererAPI::ClearRenderTarget(const Ref<Texture2D>& p_Texture, const glm::vec4& p_Value)
	{
		KTN_PROFILE_FUNCTION_LOW();

		KTN_CORE_VERIFY(p_Texture != nullptr);

		auto id = As<Texture2D, GLTexture2D>(p_Texture)->GetID();

		if (p_Texture->IsColorAttachment())
		{
			glClearTexImage(id, 0, GLUtils::TextureFormatToGLFormat(p_Texture->GetSpecification().Format), GL_FLOAT, &p_Value[0]);
		}
		else if (p_Texture->IsDepthStencilAttachment())
		{
			float value = 1.0f;
			glClearTexImage(id, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &value);

			if (p_Texture->IsStencil())
			{
				GLint clearStencil = 0;
				glClearTexImage(id, 0, GL_STENCIL_INDEX, GL_INT, &clearStencil);
			}
		}
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