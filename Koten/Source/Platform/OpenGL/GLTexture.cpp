#include "ktnpch.h"
#include "GLBase.h"
#include "GLTexture.h"
#include "GLUtils.h"
#include "Koten/Graphics/RendererCommand.h"




namespace KTN
{
	GLTexture2D::GLTexture2D(const TextureSpecification& p_Spec)
	{
		Init(p_Spec);
	}

	GLTexture2D::GLTexture2D(const TextureSpecification& p_Spec, const uint8_t* p_Data, size_t p_Size)
	{
		if (p_Spec.Samples > 1)
			KTN_CORE_WARN("Samples is just for render target textures!")

		Init(p_Spec);

		SetData(p_Data, p_Size);
	}

	GLTexture2D::~GLTexture2D()
	{
		GLCall(glDeleteTextures(1, &m_RendererID));
	}

	void GLTexture2D::Resize(uint32_t p_Width, uint32_t p_Height)
	{
		const auto& cap = RendererCommand::GetCapabilities();

		uint32_t newTexID;
		if (IsStorage())
		{
			GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &newTexID));
			GLCall(glTextureStorage2D(newTexID, 1, m_InternalFormat, p_Width, p_Height));

			GLCall(glTextureParameteri(newTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			GLCall(glTextureParameteri(newTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			GLCall(glTextureParameteri(newTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTextureParameteri(newTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

			GLCall(glDeleteTextures(1, &m_RendererID));
			m_RendererID	= newTexID;
			m_Width			= p_Width;
			m_Height		= p_Height;
			return;
		}

		if (m_Specification.Samples > 1 && IsColorAttachment())
		{
			GLCall(glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &newTexID));
			GLCall(glTextureStorage2DMultisample(newTexID, m_Specification.Samples, m_InternalFormat, p_Width, p_Height, GL_TRUE));
		}
		else
		{
			GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &newTexID));
			GLCall(glTextureStorage2D(newTexID, 1, m_InternalFormat, p_Width, p_Height));

			GLCall(glTextureParameteri(newTexID, GL_TEXTURE_WRAP_S, GLUtils::TextureWrapToGL(m_Specification.WrapU)));
			GLCall(glTextureParameteri(newTexID, GL_TEXTURE_WRAP_T, GLUtils::TextureWrapToGL(m_Specification.WrapV)));
			GLCall(glTextureParameteri(newTexID, GL_TEXTURE_MIN_FILTER, GLUtils::TextureFilterToGL(m_Specification.MinFilter, m_Specification.GenerateMips)));
			GLCall(glTextureParameteri(newTexID, GL_TEXTURE_MAG_FILTER, GLUtils::TextureFilterToGL(m_Specification.MagFilter)));

			if (m_Specification.GenerateMips)
			{
				GLCall(glGenerateTextureMipmap(newTexID));
			}

			if (m_Specification.AnisotropyEnable && cap.SamplerAnisotropy)
			{
				GLCall(glTextureParameterf(newTexID, GL_TEXTURE_MAX_ANISOTROPY, cap.MaxAnisotropy));
			}
		}

		if (m_Specification.Samples <= 1 && !IsSampled())
		{
			GLuint fboIDs[2] = { 0 };
			GLCall(glGenFramebuffers(2, fboIDs));

			GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, fboIDs[0]));
			GLCall(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RendererID, 0));

			GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboIDs[1]));
			GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newTexID, 0));

			GLCall(glBlitFramebuffer(0, 0, m_Width, m_Height, 0, 0, p_Width, p_Height, GL_COLOR_BUFFER_BIT, GL_LINEAR));

			GLCall(glDeleteFramebuffers(2, fboIDs));
		}

		GLCall(glDeleteTextures(1, &m_RendererID));

		m_RendererID	= newTexID;
		m_Width			= p_Width;
		m_Height		= p_Height;
	}

	void GLTexture2D::GenerateMipmap(CommandBuffer* p_CommandBuffer)
	{
		m_Specification.GenerateMips = true;
		
		GLCall(glGenerateTextureMipmap(m_RendererID));
		m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max({ m_Width, m_Height, 1u })))) + 1;
	}

	void GLTexture2D::Bind(uint32_t p_Slot)
	{
		m_Slot = p_Slot;
		if (IsStorage())
			GLCall(glBindImageTexture(p_Slot, m_RendererID, 0, GL_FALSE, 0, GLUtils::TextureAccessToGL(m_Specification.Access), m_InternalFormat));
		else
			GLCall(glBindTextureUnit(p_Slot, m_RendererID));
	}

	void GLTexture2D::Unbind()
	{
		if (IsStorage())
			GLCall(glBindImageTexture(m_Slot, 0, 0, GL_FALSE, 0, GLUtils::TextureAccessToGL(m_Specification.Access), m_InternalFormat));
		else
			GLCall(glBindTextureUnit(m_Slot, 0));

		m_Slot = -1;
	}

	void GLTexture2D::SetData(const void* p_Data, size_t p_Size)
	{
		KTN_CORE_ASSERT(m_Specification.Samples <= 1, "SetData is not supported for multisample textures!");
		KTN_CORE_ASSERT(p_Size >= size_t(m_Width * m_Height * m_Channels * m_BytesPerChannels), "Data must be entire texture!");
		GLCall(glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_Format, GL_UNSIGNED_BYTE, p_Data));
	}

	void GLTexture2D::Init(const TextureSpecification& p_Spec)
	{
		const auto& cap			= RendererCommand::GetCapabilities();

		m_Specification			= p_Spec;
		m_Specification.Samples = std::min(cap.MaxSamples, p_Spec.Samples);
		m_Format				= GLUtils::TextureFormatToGLFormat(p_Spec.Format);
		m_InternalFormat		= GLUtils::TextureFormatToGLInternalFormat(p_Spec.Format);
		m_Channels				= Utils::TextureFormatToChannels(p_Spec.Format);
		m_BytesPerChannels		= Utils::TextureFormatToBytesPerChannels(p_Spec.Format);
		m_Width					= p_Spec.Width;
		m_Height				= p_Spec.Height;

		if (IsStorage())
		{
			GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID));
			GLCall(glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height));

			GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

			return;
		}

		// multisampling
		if (m_Specification.Samples > 1 && IsColorAttachment())
		{
			GLCall(glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_RendererID));
			GLCall(glTextureStorage2DMultisample(m_RendererID, m_Specification.Samples, m_InternalFormat, m_Width, m_Height, GL_TRUE));
			return;
		}

		GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID));
		GLCall(glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height));

		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GLUtils::TextureWrapToGL(p_Spec.WrapU)));
		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GLUtils::TextureWrapToGL(p_Spec.WrapV)));
		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GLUtils::TextureFilterToGL(p_Spec.MinFilter, p_Spec.GenerateMips)));
		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GLUtils::TextureFilterToGL(p_Spec.MagFilter)));


		if (p_Spec.GenerateMips)
		{
			GLCall(glGenerateTextureMipmap(m_RendererID));
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max({ m_Width, m_Height, 1u })))) + 1;
		}

		if (p_Spec.AnisotropyEnable && cap.SamplerAnisotropy)
		{
			GLCall(glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, cap.MaxAnisotropy));
		}
	}

} // namespace KTN
