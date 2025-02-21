#include "ktnpch.h"
#include "GLBase.h"
#include "GLIndirectBuffer.h"



namespace KTN
{
	GLIndirectBuffer::GLIndirectBuffer(size_t p_Size)
		: m_Size(p_Size)
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glGenBuffers(1, &m_RendererID));
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID));
		GLCall(glBufferData(GL_DRAW_INDIRECT_BUFFER, p_Size, nullptr, GL_DYNAMIC_DRAW));
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
	}

	GLIndirectBuffer::GLIndirectBuffer(const void* p_Data, size_t p_Size)
		: m_Size(p_Size)
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glGenBuffers(1, &m_RendererID));
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID));
		GLCall(glBufferData(GL_DRAW_INDIRECT_BUFFER, p_Size, p_Data, GL_DYNAMIC_DRAW));
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
	}

	GLIndirectBuffer::~GLIndirectBuffer()
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glDeleteBuffers(1, &m_RendererID));
	}

	void GLIndirectBuffer::Bind()
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID));
	}

	void GLIndirectBuffer::Unbind()
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
	}

	void GLIndirectBuffer::BindBase(uint32_t p_Slot)
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_Slot = p_Slot;
		GLCall(glBindBufferBase(GL_DRAW_INDIRECT_BUFFER, p_Slot, m_RendererID));
	}

	void GLIndirectBuffer::UnbindBase()
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glBindBufferBase(GL_DRAW_INDIRECT_BUFFER, m_Slot, m_RendererID));
		m_Slot = -1;
	}

	void GLIndirectBuffer::Clear()
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID));
		GLCall(glClearBufferData(GL_DRAW_INDIRECT_BUFFER, GL_R32UI, GL_RED, GL_UNSIGNED_INT, nullptr));
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
	}

	void GLIndirectBuffer::SetData(const void* p_Data, size_t p_Size, size_t p_Offset)
	{
		KTN_PROFILE_FUNCTION_LOW();

		if ((p_Size + p_Offset) > m_Size)
			Resize(p_Size + p_Offset);

		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID));
		GLCall(glBufferSubData(GL_DRAW_INDIRECT_BUFFER, p_Offset, p_Size, p_Data));
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
	}

	void GLIndirectBuffer::Resize(size_t p_Size)
	{
		KTN_PROFILE_FUNCTION_LOW();

		if (m_Size == p_Size)
			return;

		GLuint newBuffer;
		GLCall(glGenBuffers(1, &newBuffer));
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, newBuffer));
		GLCall(glBufferData(GL_DRAW_INDIRECT_BUFFER, p_Size, nullptr, GL_DYNAMIC_DRAW));
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));

		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID));
		void* oldBufferData = glMapBuffer(GL_DRAW_INDIRECT_BUFFER, GL_READ_ONLY);
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));

		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, newBuffer));
		void* newBufferData = glMapBuffer(GL_DRAW_INDIRECT_BUFFER, GL_WRITE_ONLY);
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));

		memcpy(newBufferData, oldBufferData, std::min(m_Size, p_Size));

		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_RendererID));
		glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, newBuffer));
		glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
		GLCall(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));

		GLCall(glDeleteBuffers(1, &m_RendererID));
		m_RendererID = newBuffer;
		m_Size = p_Size;
	}


} // namespace KTN