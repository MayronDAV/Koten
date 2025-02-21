#pragma once
#include "StorageBuffer.h"



namespace KTN
{
	class KTN_API IndirectBuffer : public StorageBuffer
	{
		public:
			virtual void Clear() = 0;

			StorageBufferType GetType() override { return StorageBufferType::Indirect; }

			static Ref<IndirectBuffer> Create(size_t p_Size); // DYNAMIC usage
			static Ref<IndirectBuffer> Create(const void* p_Data, size_t p_Size); // STATIC usage
	};

} // namespace KTN
