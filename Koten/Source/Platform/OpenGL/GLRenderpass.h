#pragma once
#include "Koten/Graphics/Renderpass.h"
#include "GLFramebuffer.h"


namespace KTN
{
	class GLRenderpass : public Renderpass
	{
		public:
			GLRenderpass(const RenderpassSpecification& p_Spec);
			~GLRenderpass() override;

			void Begin(CommandBuffer* p_CommandBuffer, const Ref<Framebuffer>& p_Frame, uint32_t p_Width, uint32_t p_Height, const glm::vec4& p_Color = { 1, 1, 1, 1 }, SubpassContents p_Contents = SubpassContents::INLINE) override;
			void End(CommandBuffer* p_CommandBuffer) override;
			int GetAttachmentCount() const override { return (int)m_Spec.AttachmentCount; }

			const RenderpassSpecification& GetSpecification() const override { return m_Spec; }

		private:
			Ref<GLFramebuffer> m_CurrentFrame = nullptr;
			RenderpassSpecification m_Spec = {};
	};

} // namespace KTN