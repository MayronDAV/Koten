#include "ktnpch.h"
#include "GLBase.h"
#include "GLBuffer.h"




namespace KTN
{
	#pragma region GLVertexBuffer

	GLVertexBuffer::GLVertexBuffer(size_t p_Size)
	{
		GLCall(glCreateBuffers(1, &m_RendererID));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
		GLCall(glBufferData(GL_ARRAY_BUFFER, p_Size, nullptr, GL_DYNAMIC_DRAW));
	}

	GLVertexBuffer::GLVertexBuffer(const void* p_Data, size_t p_Size)
	{
		GLCall(glCreateBuffers(1, &m_RendererID));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
		GLCall(glBufferData(GL_ARRAY_BUFFER, p_Size, p_Data, GL_STATIC_DRAW));
	}

	GLVertexBuffer::~GLVertexBuffer()
	{
		GLCall(glDeleteBuffers(1, &m_RendererID));
	}

	void GLVertexBuffer::Bind(CommandBuffer* p_CommandBuffer) const
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
	}

	void GLVertexBuffer::Unbind() const
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	void GLVertexBuffer::SetData(const void* p_Data, size_t p_Size, size_t p_Offset)
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
		GLCall(glBufferSubData(GL_ARRAY_BUFFER, p_Offset, p_Size, p_Data));
	}

	#pragma endregion

	#pragma region GLIndexBuffer

	GLIndexBuffer::GLIndexBuffer(uint32_t* p_Indices, uint32_t p_Count)
		: m_Count(p_Count)
	{
		GLCall(glCreateBuffers(1, &m_RendererID));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Count * sizeof(uint32_t), p_Indices, GL_STATIC_DRAW));
	}

	GLIndexBuffer::~GLIndexBuffer()
	{
		GLCall(glDeleteBuffers(1, &m_RendererID));
	}

	void GLIndexBuffer::Bind(CommandBuffer* p_CommandBuffer) const
	{
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID));
	}

	void GLIndexBuffer::Unbind() const
	{
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}

	#pragma endregion


} // namespace KTN