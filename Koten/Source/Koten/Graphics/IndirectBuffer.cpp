#include "ktnpch.h"
#include "IndirectBuffer.h"

#include "Platform/OpenGL/GLIndirectBuffer.h"


namespace KTN
{
	Ref<IndirectBuffer> IndirectBuffer::Create(size_t p_Size)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLIndirectBuffer>(p_Size);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<IndirectBuffer> IndirectBuffer::Create(const void* p_Data, size_t p_Size)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLIndirectBuffer>(p_Data, p_Size);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}
} // namespace KTN