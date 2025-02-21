#include "ktnpch.h"
#include "GLBase.h"
#include "GLStorageBuffer.h"



namespace KTN
{
	#define CHECK_ALIGNMENT(size) 																							\
	{																														\
		GLint alignment; 																									\
		GLCall(glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &alignment)); 										\
		KTN_CORE_ASSERT(size % alignment == 0, "Uniform Buffer size must be aligned to " + std::to_string((int)alignment)); \
	}

	GLStorageBuffer::GLStorageBuffer(size_t p_Size)
	{
		KTN_PROFILE_FUNCTION_LOW();

		CHECK_ALIGNMENT(p_Size);

		GLCall(glCreateBuffers(1, &m_RendererID));
		GLCall(glNamedBufferStorage(m_RendererID, p_Size, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT));
	}

	GLStorageBuffer::GLStorageBuffer(const void* p_Data, size_t p_Size)
	{
		KTN_PROFILE_FUNCTION_LOW();

		CHECK_ALIGNMENT(p_Size);

		GLCall(glCreateBuffers(1, &m_RendererID));
		GLCall(glNamedBufferStorage(m_RendererID, p_Size, p_Data, 0));
	}

	GLStorageBuffer::~GLStorageBuffer()
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glDeleteBuffers(1, &m_RendererID));
	}

	void GLStorageBuffer::SetData(const void* p_Data, size_t p_Size, size_t p_Offset)
	{
		KTN_PROFILE_FUNCTION_LOW();

		CHECK_ALIGNMENT(p_Size);

		if ((p_Size + p_Offset) > m_Size)
		{
			Resize(p_Size + p_Offset);
		}

		GLCall(glNamedBufferSubData(m_RendererID, (GLintptr)p_Offset, (GLsizeiptr)p_Size, p_Data));
	}

	void GLStorageBuffer::Resize(size_t p_Size)
	{
		KTN_PROFILE_FUNCTION_LOW();

		if (m_Size == p_Size)
			return;

		GLuint newBuffer;
		glCreateBuffers(1, &newBuffer);
		glNamedBufferStorage(newBuffer, p_Size, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);

		void* oldBufferData = glMapNamedBuffer(m_RendererID, GL_READ_ONLY);
		void* newBufferData = glMapNamedBuffer(newBuffer, GL_WRITE_ONLY);

		memcpy(newBufferData, oldBufferData, std::min(m_Size, p_Size));

		glUnmapNamedBuffer(m_RendererID);
		glUnmapNamedBuffer(newBuffer);

		glDeleteBuffers(1, &m_RendererID);
		m_RendererID = newBuffer;
		m_Size = p_Size;
	}

	void GLStorageBuffer::Bind(uint32_t p_Slot)
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_Slot = p_Slot;
		GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_Slot, m_RendererID));
	}

	void GLStorageBuffer::Unbind()
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_Slot, 0));
		m_Slot = -1;
	}

} // namespace KTN
