#include "ktnpch.h"
#include "GLBase.h"
#include "GLCommandBuffer.h"



namespace KTN
{
	void GLCommandBuffer::SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height)
	{
		GLCall(glViewport((GLint)p_X, (GLint)p_Y, (GLsizei)p_Width, (GLsizei)p_Height));
	}

	void GLCommandBuffer::DispatchCompute(uint32_t p_NumGroups_X, uint32_t p_NumGroups_Y, uint32_t p_NumGroups_Z)
	{
		GLCall(glDispatchCompute(p_NumGroups_X, p_NumGroups_Y, p_NumGroups_Z));
	}

} // namespace KTN
