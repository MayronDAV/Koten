#pragma once
#include "Koten/Graphics/IndirectBuffer.h"



namespace KTN
{
	class GLIndirectBuffer : public IndirectBuffer
	{
		public:
			GLIndirectBuffer(size_t p_Size);
			GLIndirectBuffer(const void* p_Data, size_t p_Size);
			~GLIndirectBuffer() override;

			uint32_t GetID() const { return m_RendererID; }

			void Bind();
			void Unbind();

			void BindBase(uint32_t p_Slot);
			void UnbindBase();

			void Clear() override;

			void SetData(const void* p_Data, size_t p_Size, size_t p_Offset = 0ul) override;
			void Resize(size_t p_Size) override;

		private:
			uint32_t m_Slot = -1;
			uint32_t m_RendererID = 0;
			size_t m_Size = 0ul;
	};

} // namespace KTN
