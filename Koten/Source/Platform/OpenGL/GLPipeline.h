#pragma once

#include "Koten/Graphics/Pipeline.h"



namespace KTN
{
	class GLPipeline : public Pipeline
	{
		public:
			GLPipeline(const PipelineSpecification& p_Spec);
			~GLPipeline();

			void Begin(CommandBuffer* p_CommandBuffer, SubpassContents p_Contents, int p_MipIndex) override;
			void End(CommandBuffer* p_CommandBuffer) override;

			const Ref<Renderpass>& GetRenderpass() const override { return m_Renderpass; }
			const Ref<Framebuffer>& GetFramebuffer(int p_MipIndex = 0) const override;

			const Ref<Shader>& GetShader() override { return m_Shader; }

		private:
			void CreateFramebuffers();

		private:
			Ref<Shader> m_Shader = nullptr;
			Ref<Renderpass> m_Renderpass;
			std::vector<Ref<Framebuffer>> m_Framebuffers;
	};
} // namespace KTN
