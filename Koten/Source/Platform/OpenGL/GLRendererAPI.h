#pragma once
#include "Koten/Graphics/RendererAPI.h"



namespace KTN
{
	class GLRendererAPI : public RendererAPI
	{
		public:
			GLRendererAPI();
			~GLRendererAPI() override = default;

			void ClearColor(const glm::vec4& p_Color) override;
			void ClearRenderTarget(const Ref<Texture2D>& p_Texture, int p_Value) override;
			void ClearRenderTarget(const Ref<Texture2D>& p_Texture, const glm::vec4& p_Value) override;

			void* ReadPixel(const Ref<Texture2D>& p_Texture, uint32_t p_X, uint32_t p_Y) override;

			void Begin() override;
			void End() override;

			const Capabilities& GetCapabilities() const override { return m_Capabilities; }

			CommandBuffer* GetCurrentCommandBuffer() override { return m_CommandBuffer.get(); }

		private:
			Capabilities m_Capabilities = {};
			Unique<CommandBuffer> m_CommandBuffer = nullptr;
			uint32_t m_FBO = 0;
	};


} // namespace KTN