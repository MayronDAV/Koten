#include "ktnpch.h"
#include "UniformBuffer.h"

#include "Platform/OpenGL/GLUniformBuffer.h"



namespace KTN
{
	Ref<UniformBuffer> UniformBuffer::Create(size_t p_SizeBytes)
	{
		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLUniformBuffer>(p_SizeBytes);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<UniformBuffer> UniformBuffer::Create(const void* p_Data, size_t p_SizeBytes)
	{
		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLUniformBuffer>(p_Data, p_SizeBytes);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

} // namespace KTN