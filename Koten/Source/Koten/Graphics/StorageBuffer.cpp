#include "ktnpch.h"
#include "StorageBuffer.h"

#include "Platform/OpenGL/GLStorageBuffer.h"



namespace KTN
{
	Ref<StorageBuffer> StorageBuffer::Create(size_t p_Size)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLStorageBuffer>(p_Size);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<StorageBuffer> StorageBuffer::Create(const void* p_Data, size_t p_Size)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLStorageBuffer>(p_Data, p_Size);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

} // namespace KTN