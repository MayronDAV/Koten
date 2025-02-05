#pragma once
#include "Koten/Graphics/Buffer.h"


namespace KTN
{
	class GLVertexBuffer : public VertexBuffer
	{
		public:
			GLVertexBuffer(size_t p_Size);
			GLVertexBuffer(const void* p_Data, size_t p_Size);
			~GLVertexBuffer() override;

			void Bind(CommandBuffer* p_CommandBuffer) const override;
			void Unbind() const override;

			void SetData(const void* p_Data, size_t p_Size, size_t p_Offset) override;

			const BufferLayout& GetLayout() const override { return m_Layout; }
			void SetLayout(const BufferLayout& p_Layout) override { m_Layout = p_Layout; }

			uint32_t GetID() const { return m_RendererID; }

		private:
			uint32_t m_RendererID = 0;
			BufferLayout m_Layout;
	};

	class GLIndexBuffer : public IndexBuffer
	{
		public:
			GLIndexBuffer(uint32_t* p_Indices, uint32_t p_Count);
			~GLIndexBuffer() override;

			void Bind(CommandBuffer* p_CommandBuffer) const override;
			void Unbind() const override;

			uint32_t GetCount() const override { return m_Count; }

			uint32_t GetID() const { return m_RendererID; }
		
		private:
			uint32_t m_RendererID = 0;
			uint32_t m_Count = 0;
	};

} // namespace KTN
