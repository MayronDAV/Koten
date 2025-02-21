#pragma once
#include "Koten/Core/Base.h"



namespace KTN
{
	enum class StorageBufferType
	{
		Storage = 0,
		Indirect
	};

	class KTN_API StorageBuffer
	{
		public:
			virtual ~StorageBuffer() = default;
			virtual void SetData(const void* p_Data, size_t p_Size, size_t p_Offset = 0) = 0;

			virtual void Resize(size_t p_Size) = 0;

			virtual StorageBufferType GetType() { return StorageBufferType::Storage; }

			static Ref<StorageBuffer> Create(size_t p_Size); // DYNAMIC usage
			static Ref<StorageBuffer> Create(const void* p_Data, size_t p_Size); // STATIC usage
	};
} // namespace KTN