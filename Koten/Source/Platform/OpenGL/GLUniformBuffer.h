#pragma once
#include "Koten/Graphics/UniformBuffer.h"



namespace KTN
{
	class GLUniformBuffer : public UniformBuffer
	{
		public:
			GLUniformBuffer(size_t p_SizeBytes);
			GLUniformBuffer(const void* p_Data, size_t p_SizeBytes);
			~GLUniformBuffer() override;

			void SetData(const void* p_Data, size_t p_SizeBytes, size_t p_Offset = 0) override;

			void Bind(uint32_t p_Slot);
			void Unbind();

			uint32_t GetID() const { return m_RendererID; }

		private:
			uint32_t m_RendererID	= 0;
			uint32_t m_Slot			= -1;
	};
}