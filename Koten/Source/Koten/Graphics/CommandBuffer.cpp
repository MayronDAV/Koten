#include "ktnpch.h"
#include "CommandBuffer.h"

#include "Platform/OpenGL/GLCommandBuffer.h"



namespace KTN
{
	void CommandBuffer::BindSets(const Ref<DescriptorSet>* p_Sets, uint32_t p_Count)
	{
		KTN_PROFILE_FUNCTION();

		for (uint32_t i = 0; i < p_Count; i++)
		{
			if (p_Sets[i])
				p_Sets[i]->Bind(this);
		}
	}

	Unique<CommandBuffer> CommandBuffer::Create()
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateUnique<GLCommandBuffer>();

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

} // namespace KTN