#include "ktnpch.h"
#include "Buffer.h"

#include "Platform/OpenGL/GLBuffer.h"



namespace KTN
{
	Ref<VertexBuffer> VertexBuffer::Create(size_t p_Size)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLVertexBuffer>(p_Size);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(const void* p_Data, size_t p_Size)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLVertexBuffer>(p_Data, p_Size);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* p_Indices, uint32_t p_Count)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLIndexBuffer>(p_Indices, p_Count);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}
}
