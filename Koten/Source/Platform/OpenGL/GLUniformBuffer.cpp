#include "ktnpch.h"
#include "GLBase.h"
#include "GLUniformBuffer.h"



namespace KTN
{
	#define CHECK_ALIGNMENT(size) 																							\
	{																														\
		GLint alignment; 																									\
		GLCall(glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment));  											\
		KTN_CORE_ASSERT(size % alignment == 0, "Uniform Buffer size must be aligned to " + std::to_string((int)alignment)); \
	}

	GLUniformBuffer::GLUniformBuffer(size_t p_SizeBytes)
	{
		KTN_PROFILE_FUNCTION_LOW();

		CHECK_ALIGNMENT(p_SizeBytes);

		GLCall(glCreateBuffers(1, &m_RendererID));
		GLCall(glNamedBufferData(m_RendererID, p_SizeBytes, nullptr, GL_DYNAMIC_DRAW));
	}

	GLUniformBuffer::GLUniformBuffer(const void* p_Data, size_t p_SizeBytes)
	{
		KTN_PROFILE_FUNCTION_LOW();

		CHECK_ALIGNMENT(p_SizeBytes);

		GLCall(glCreateBuffers(1, &m_RendererID));
		GLCall(glNamedBufferData(m_RendererID, p_SizeBytes, p_Data, GL_STATIC_DRAW));
	}

	GLUniformBuffer::~GLUniformBuffer()
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glDeleteBuffers(1, &m_RendererID));
	}

	void GLUniformBuffer::SetData(const void* p_Data, size_t p_SizeBytes, size_t p_Offset)
	{
		KTN_PROFILE_FUNCTION_LOW();

		CHECK_ALIGNMENT(p_SizeBytes);
		GLCall(glNamedBufferSubData(m_RendererID, (GLintptr)p_Offset, (GLsizeiptr)p_SizeBytes, p_Data));
	}

	void GLUniformBuffer::Bind(uint32_t p_Slot)
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_Slot = p_Slot;
		GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_Slot, m_RendererID));
	}

	void GLUniformBuffer::Unbind()
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, m_Slot, 0));
		m_Slot = -1;
	}

} // namespace KTN