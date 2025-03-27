#pragma once
#include "Koten/Graphics/Framebuffer.h"



namespace KTN
{
	class GLFramebuffer : public Framebuffer
	{
		public:
			GLFramebuffer(const FramebufferSpecification& p_Spec);
			~GLFramebuffer() override;

			void Validate() override;

			void Begin() const;
			void End() const;

			uint32_t GetWidth() const override { return m_Spec.Width; }
			uint32_t GetHeight() const override { return m_Spec.Height; }
			uint32_t GetID() const { return m_RendererID; }

		private:
			void Init();

		private:
			uint32_t m_RendererID			= 0;
			uint32_t m_ResolveID			= -1;
			uint32_t m_ColorAttachmentCount = 0;
			FramebufferSpecification m_Spec = {};
	};
}