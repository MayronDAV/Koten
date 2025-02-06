#pragma once
#include "Koten/Core/Base.h"



namespace KTN
{
	class KTN_API UniformBuffer
	{
		public:
			virtual ~UniformBuffer() = default;
			virtual void SetData(const void* p_Data, size_t p_SizeBytes, size_t p_Offset = 0) = 0;

			static Ref<UniformBuffer> Create(size_t p_SizeBytes); // DYNAMIC usage
			static Ref<UniformBuffer> Create(const void* p_Data, size_t p_SizeBytes); // STATIC usage
	};
}