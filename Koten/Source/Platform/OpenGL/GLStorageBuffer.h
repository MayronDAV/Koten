#pragma once
#include "Koten/Graphics/StorageBuffer.h"



namespace KTN
{
	class GLStorageBuffer : public StorageBuffer
	{
		public:
			GLStorageBuffer(size_t p_Size);
			GLStorageBuffer(const void* p_Data, size_t p_Size);
			~GLStorageBuffer();
			void SetData(const void* p_Data, size_t p_Size, size_t p_Offset = 0) override;

			void Resize(size_t p_Size) override;

			uint32_t GetID() const { return m_RendererID; }
			void Bind(uint32_t p_Slot);
			void Unbind();

		private:
			uint32_t m_RendererID = 0;
			uint32_t m_Slot = -1;
			size_t m_Size = 0ul;
	};

} // namespace KTN