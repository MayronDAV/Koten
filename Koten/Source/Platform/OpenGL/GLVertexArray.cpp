#include "ktnpch.h"
#include "GLBase.h"
#include "GLVertexArray.h"
#include "Koten/Utils/Utils.h"



namespace KTN
{
	namespace
	{
		static GLenum DataTypeToOpenGLBaseType(DataType p_Type)
		{
			switch (p_Type)
			{
				case DataType::Float:		return GL_FLOAT;
				case DataType::Float2:		return GL_FLOAT;
				case DataType::Float3:		return GL_FLOAT;
				case DataType::Float4:		return GL_FLOAT;
				case DataType::Float3x3:	return GL_FLOAT;
				case DataType::Float4x4:	return GL_FLOAT;
				case DataType::Int:			return GL_INT;
				case DataType::Int2:		return GL_INT;
				case DataType::Int3:		return GL_INT;
				case DataType::Int4:		return GL_INT;
				case DataType::Int3x3:		return GL_INT;
				case DataType::Int4x4:		return GL_INT;
				case DataType::Bool:		return GL_BOOL;
			}

			KTN_CORE_ERROR("Unknown DataType!");
			return 0;
		}

		static void SetMatrixAttrib(DataType p_BaseType, uint32_t& p_VertexBufferIndex,  const BufferElement& p_Element, uint32_t p_Stride)
		{
			auto count = p_Element.GetComponentCount();
			for (uint8_t i = 0; i < count; i++)
			{
				glEnableVertexAttribArray(p_VertexBufferIndex);
				glVertexAttribPointer(p_VertexBufferIndex,
					count,
					DataTypeToOpenGLBaseType(p_Element.Type),
					p_Element.Normalized ? GL_TRUE : GL_FALSE,
					p_Stride,
					(const void*)(const uint64_t)(p_Element.Offset + Utils::DataTypeSize(p_BaseType) * count * i));
				glVertexAttribDivisor(p_VertexBufferIndex, 1);
				p_VertexBufferIndex++;
			}
		}

	} // namespace

	GLVertexArray::GLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	GLVertexArray::~GLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void GLVertexArray::Bind(CommandBuffer* p_CommandBuffer) const
	{
		KTN_PROFILE_FUNCTION_LOW();

		glBindVertexArray(m_RendererID);
	}

	void GLVertexArray::Unbind() const
	{
		KTN_PROFILE_FUNCTION_LOW();

		glBindVertexArray(0);
	}

	void GLVertexArray::SetVertexBuffer(const Ref<VertexBuffer>& p_VertexBuffer)
	{
		KTN_PROFILE_FUNCTION_LOW();

		KTN_CORE_ASSERT(p_VertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_RendererID);
		p_VertexBuffer->Bind(nullptr);

		const auto& layout = p_VertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			uint8_t count = element.GetComponentCount();
			switch (element.Type)
			{
				case DataType::Float:
				case DataType::Float2:
				case DataType::Float3:
				case DataType::Float4:
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex,
						count,
						DataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)element.Offset);
					m_VertexBufferIndex++;
					break;
				}
				case DataType::Int:
				case DataType::Int2:
				case DataType::Int3:
				case DataType::Int4:
				case DataType::Bool:
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribIPointer(m_VertexBufferIndex,
						count,
						DataTypeToOpenGLBaseType(element.Type),
						layout.GetStride(),
						(const void*)element.Offset);
					m_VertexBufferIndex++;
					break;
				}
				case DataType::Float3x3:
				case DataType::Float4x4:
				{
					SetMatrixAttrib(DataType::Float, m_VertexBufferIndex, element, layout.GetStride());
					break;
				}
				case DataType::Int3x3:
				case DataType::Int4x4:
				{
					SetMatrixAttrib(DataType::Int, m_VertexBufferIndex, element, layout.GetStride());
					break;
				}
				default:
					KTN_GLERROR("Unknown DataType!");
					break;
			}
		}

		m_VertexBuffer = p_VertexBuffer;
	}

	void GLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& p_IndexBuffer)
	{
		KTN_PROFILE_FUNCTION_LOW();

		glBindVertexArray(m_RendererID);
		p_IndexBuffer->Bind(nullptr);

		m_IndexBuffer = p_IndexBuffer;
	}

} // namespace KTN
