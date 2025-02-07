#include "ktnpch.h"
#include "GLBase.h"
#include "GLRenderpass.h"
#include "GLFramebuffer.h"



namespace KTN
{
	GLRenderpass::GLRenderpass(const RenderpassSpecification& p_Spec)
		: m_Spec(p_Spec)
	{
	}

	GLRenderpass::~GLRenderpass()
	{
	}

	void GLRenderpass::Begin(CommandBuffer* p_CommandBuffer, const Ref<Framebuffer>& p_Frame, uint32_t p_Width, uint32_t p_Height, const glm::vec4& p_Color, SubpassContents p_Contents)
	{
		if (p_Frame != nullptr)
		{
			m_CurrentFrame = As<Framebuffer, GLFramebuffer>(p_Frame);
			m_CurrentFrame->Begin();
		}
		else
		{
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		}

		GLCall(glViewport(0, 0, p_Width, p_Height));

		if (m_Spec.Clear)
		{
			GLCall(glClearColor(p_Color.r, p_Color.g, p_Color.b, p_Color.a));
			GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
		}
	}

	void GLRenderpass::End(CommandBuffer* p_CommandBuffer)
	{
		if (m_CurrentFrame)
		{
			m_CurrentFrame->End();
		}

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

} // namespace KTN
