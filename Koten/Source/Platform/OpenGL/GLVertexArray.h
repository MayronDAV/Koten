#pragma once
#include "Koten/Graphics/VertexArray.h"



namespace KTN
{
	class GLVertexArray : public VertexArray
	{
		public:
			GLVertexArray();
			~GLVertexArray() override;

			void Bind(CommandBuffer* p_CommandBuffer) const override;
			void Unbind() const override;

			void SetVertexBuffer(const Ref<VertexBuffer>& p_VertexBuffer) override;
			void SetIndexBuffer(const Ref<IndexBuffer>& p_IndexBuffer) override;

			const Ref<VertexBuffer>& GetVertexBuffer() const override { return m_VertexBuffer; }
			const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

			uint32_t GetID() const { return m_RendererID; }

		private:
			uint32_t m_RendererID				= 0;
			Ref<VertexBuffer> m_VertexBuffer	= nullptr;
			Ref<IndexBuffer> m_IndexBuffer		= nullptr;
			uint32_t m_VertexBufferIndex		= 0;
	};

} // namespace KTN
