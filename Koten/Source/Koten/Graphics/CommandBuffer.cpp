#include "ktnpch.h"
#include "CommandBuffer.h"

#include "Platform/OpenGL/GLCommandBuffer.h"



namespace KTN
{
	Unique<CommandBuffer> CommandBuffer::Create()
	{
		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateUnique<GLCommandBuffer>();

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

} // namespace KTN