#include "ktnpch.h"
#include "GLBase.h"
#include "GLUtils.h"
#include "GLPipeline.h"




namespace KTN
{
	GLPipeline::GLPipeline(const PipelineSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_Spec = p_Spec;
		m_Shader = p_Spec.pShader;

		CreateFramebuffers();
	}

	GLPipeline::~GLPipeline()
	{
	}

	void GLPipeline::Begin(CommandBuffer* p_CommandBuffer, SubpassContents p_Contents, int p_MipIndex)
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_Renderpass->Begin(p_CommandBuffer, GetFramebuffer(), GetWidth(), GetHeight(), m_Spec.ClearColor, p_Contents);

		m_Shader->Bind();

		if (m_Spec.DepthTest)
			GLCall(glEnable(GL_DEPTH_TEST));
		else
			GLCall(glDisable(GL_DEPTH_TEST));

		if (m_Spec.DepthWrite)
			GLCall(glDepthMask(GL_TRUE));
		else
			GLCall(glDepthMask(GL_FALSE));

		if (m_Spec.DepthBiasEnabled)
		{
			GLCall(glEnable(GL_POLYGON_OFFSET_FILL));
			p_CommandBuffer->SetDepthBias(m_Spec.ConstantFactor, m_Spec.SlopeFactor);
		}
		else
			GLCall(glDisable(GL_POLYGON_OFFSET_FILL));

		if (m_Spec.StencilTest)
		{
			GLCall(glEnable(GL_STENCIL_TEST));
			p_CommandBuffer->SetStencil(
				m_Spec.pStencilFace, m_Spec.pStencilCompare,
				m_Spec.StencilCompareMask, m_Spec.StencilWriteMask, 
				m_Spec.StencilReference);
			GLCall(glStencilOpSeparate(
				GLUtils::StencilFaceToGL(m_Spec.pStencilFace),
				GLUtils::StencilOpToGL(m_Spec.FailOp),
				GLUtils::StencilOpToGL(m_Spec.DepthFailOp),
				GLUtils::StencilOpToGL(m_Spec.PassOp)));
		}
		else
			GLCall(glDisable(GL_STENCIL_TEST));

		if (m_Spec.pCullMode != CullMode::NONE)
		{
			GLCall(glEnable(GL_CULL_FACE));
			GLCall(glCullFace(GLUtils::CullModeToGL(m_Spec.pCullMode)));
			GLCall(glFrontFace(GLUtils::FrontFaceToGL(m_Spec.pFrontFace)));
		}

		GLCall(glPolygonMode(GL_FRONT_AND_BACK, GLUtils::PolygonModeToGL(m_Spec.pPolygonMode)));
	}

	void GLPipeline::End(CommandBuffer* p_CommandBuffer)
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_Renderpass->End(p_CommandBuffer);

		m_Shader->Unbind();

		GLCall(glDepthMask(GL_TRUE));
		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glDisable(GL_POLYGON_OFFSET_FILL));
		GLCall(glDisable(GL_STENCIL_TEST));
		GLCall(glDisable(GL_CULL_FACE));
	}

	const Ref<Framebuffer>& GLPipeline::GetFramebuffer(int p_MipIndex) const
	{
		KTN_PROFILE_FUNCTION_LOW();

		if (m_Spec.BuildMipFramebuffers)
			return m_Framebuffers[p_MipIndex];

		return m_Framebuffers[0];
	}

	void GLPipeline::CreateFramebuffers()
	{
		KTN_PROFILE_FUNCTION_LOW();

		std::vector<Ref<Texture2D>> attachments;

		if (!m_Spec.SwapchainTarget)
		{
			for (auto& texture : m_Spec.ColorTargets)
			{
				if (texture)
				{
					attachments.push_back(texture);
				}
			}

			if (m_Spec.DepthTarget)
			{
				attachments.push_back(m_Spec.DepthTarget);
			}
		}

		RenderpassSpecification rspec	= {};
		rspec.AttachmentCount			= (uint32_t)attachments.size();
		rspec.Attachments				= attachments.data();
		rspec.ResolveTexture			= m_Spec.ResolveTexture;
		rspec.Clear						= m_Spec.ClearTargets;
		rspec.Samples					= m_Spec.Samples;
		rspec.SwapchainTarget			= m_Spec.SwapchainTarget;
		rspec.DebugName					= m_Spec.DebugName + "- GLRenderpass";

		m_Renderpass					= Renderpass::Get(rspec);


		FramebufferSpecification fspec	= {};
		fspec.Width						= GetWidth();
		fspec.Height					= GetHeight();
		fspec.RenderPass				= m_Renderpass;
		fspec.AttachmentCount			= (uint32_t)attachments.size();
		fspec.Attachments				= attachments.data();
		fspec.Samples					= m_Spec.Samples;
		fspec.SwapchainTarget			= m_Spec.SwapchainTarget;
		fspec.DebugName					= m_Spec.DebugName + " - GLFramebuffer";
		if (m_Spec.BuildMipFramebuffers && !m_Spec.SwapchainTarget)
		{
			// Maybe change this?
			uint32_t mips = attachments[0]->GetMipLevels();

			for (uint32_t i = 0; i < mips; i++)
			{
				fspec.MipIndex			= i;

				m_Framebuffers.push_back(Framebuffer::Get(fspec));
			}
		}
		else
		{
			fspec.MipIndex				= 0;

			m_Framebuffers.push_back(Framebuffer::Get(fspec));
		}
	}

} // namespace KTN