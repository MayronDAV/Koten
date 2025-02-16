#include "ktnpch.h"
#include "GLBase.h"
#include "GLFramebuffer.h"
#include "GLTexture.h"




namespace KTN
{
	namespace
	{
		static std::string FBStatusToString(uint32_t p_Status)
		{
			switch (p_Status)
			{
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:			return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:			return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
				case GL_FRAMEBUFFER_UNSUPPORTED:					return "GL_FRAMEBUFFER_UNSUPPORTED";
				case GL_FRAMEBUFFER_COMPLETE:						return "GL_FRAMEBUFFER_COMPLETE";
			}

			KTN_CORE_WARN("Unknown framebuffer status!");
			return "";
		}

	} // namespace

	GLFramebuffer::GLFramebuffer(const FramebufferSpecification& p_Spec)
		: m_Spec(p_Spec)
	{
		KTN_PROFILE_FUNCTION_LOW();

		if (m_Spec.SwapchainTarget)
		{
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		}
		else
		{
			Init();
		}
	}

	GLFramebuffer::~GLFramebuffer()
	{
		KTN_PROFILE_FUNCTION_LOW();

		if (!m_Spec.SwapchainTarget)
			GLCall(glDeleteFramebuffers(1, &m_RendererID));

		if (m_ResolveID != -1)
			GLCall(glDeleteFramebuffers(1, &m_ResolveID));
	}

	void GLFramebuffer::Validate()
	{
		KTN_PROFILE_FUNCTION_LOW();

		uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			KTN_CORE_ERROR("Unable to create Framebuffer: {}", FBStatusToString(status));
			KTN_DEBUGBREAK();
		}
	}

	void GLFramebuffer::Begin() const
	{
		KTN_PROFILE_FUNCTION_LOW();

		if (m_Spec.SwapchainTarget)
		{
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			return;
		}

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));
	}

	void GLFramebuffer::End() const
	{
		KTN_PROFILE_FUNCTION_LOW();

		if (m_ResolveID != -1)
		{
			GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererID));
			GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ResolveID));
			glBlitFramebuffer(
				0, 0, m_Spec.Width, m_Spec.Height,
				0, 0, m_Spec.Width, m_Spec.Height,
				GL_COLOR_BUFFER_BIT, GL_NEAREST
			);
		}

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	void GLFramebuffer::Init()
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glGenFramebuffers(1, &m_RendererID));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));

		m_ColorAttachmentCount = 0;

		for (uint32_t i = 0; i < m_Spec.AttachmentCount; i++)
		{
			auto& texture = m_Spec.Attachments[i];
			if (texture)
			{
				auto id = As<Texture2D, GLTexture2D>(texture)->GetID();

				if (texture->IsColorAttachment())
				{
					GLCall(glFramebufferTexture2D(
						GL_FRAMEBUFFER,
						GL_COLOR_ATTACHMENT0 + m_ColorAttachmentCount,
						m_Spec.Samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
						id,
						m_Spec.MipIndex));
					m_ColorAttachmentCount++;
				}
				else if (texture->IsDepthStencilAttachment())
				{
					GLenum type = GL_DEPTH_ATTACHMENT;
					if (texture->IsDepthStencil())
						type = GL_DEPTH_STENCIL_ATTACHMENT;
					else if (texture->IsStencil())
						type = GL_STENCIL_ATTACHMENT;

					GLCall(glFramebufferTexture2D(
						GL_FRAMEBUFFER,
						type,
						m_Spec.Samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
						id,
						0));
				}
			}
		}

		Validate();

		const auto& rspec = m_Spec.RenderPass->GetSpecification();
		if (m_Spec.Samples > 1 && rspec.ResolveTexture)
		{
			auto id = As<Texture2D, GLTexture2D>(rspec.ResolveTexture)->GetID();

			GLCall(glGenFramebuffers(1, &m_ResolveID));
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_ResolveID));

			GLCall(glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D,
				id,
				m_Spec.MipIndex));

			Validate();
		}
	}


} // namespace KTN
