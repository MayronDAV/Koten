#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"
#include "Koten/Utils/Utils.h"



namespace KTN
{
	struct KTN_API BufferElement
	{
		std::string Name	= "";
		DataType Type		= DataType::None;
		uint32_t Size		= 0u;
		size_t Offset		= 0ul;
		bool Normalized		= false;

		BufferElement() = default;
		BufferElement(DataType p_Type, const std::string& p_Name, bool p_Normalized = false)
			: Name(p_Name), Type(p_Type), Size(Utils::DataTypeSize(p_Type)), Offset(0), Normalized(p_Normalized)
		{
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case DataType::Float:		return 1;
				case DataType::Float2:		return 2;
				case DataType::Float3:		return 3;
				case DataType::Float4:		return 4;
				case DataType::Float3x3:	return 3;
				case DataType::Float4x4:	return 4;
				case DataType::Int:			return 1;
				case DataType::Int2:		return 2;
				case DataType::Int3:		return 3;
				case DataType::Int4:		return 4;
				case DataType::Int3x3:		return 3;
				case DataType::Int4x4:		return 4;
				case DataType::Bool:		return 1;
			}

			KTN_CORE_ERROR("Unknown DataType!");
			return 0;
		}
	};

	class KTN_API BufferLayout
	{
		public:
			BufferLayout() {}

			BufferLayout(std::initializer_list<BufferElement> p_Elements)
				: m_Elements(p_Elements)
			{
				CalculateOffsetsAndStride();
			}

			uint32_t GetStride() const { return m_Stride; }
			const std::vector<BufferElement>& GetElements() const { return m_Elements; }

			std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
			std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
			std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
			std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

		private:
			void CalculateOffsetsAndStride()
			{
				size_t offset = 0;
				m_Stride = 0;
				for (auto& element : m_Elements)
				{
					element.Offset = offset;
					offset += element.Size;
					m_Stride += element.Size;
				}
			}

		private:
			std::vector<BufferElement> m_Elements;
			uint32_t m_Stride = 0;
	};

	class KTN_API VertexBuffer
	{
		public:
			virtual ~VertexBuffer() = default;

			virtual void Bind(CommandBuffer* p_CommandBuffer) const = 0;
			virtual void Unbind() const = 0;

			virtual void SetData(const void* p_Data, size_t p_Size, size_t p_Offset) = 0;

			virtual const BufferLayout& GetLayout() const = 0;
			virtual void SetLayout(const BufferLayout& p_Layout) = 0;

			static Ref<VertexBuffer> Create(size_t p_Size);
			static Ref<VertexBuffer> Create(const void* p_Data, size_t p_Size);
	};

	class KTN_API IndexBuffer
	{
		public:
			virtual ~IndexBuffer() = default;

			virtual void Bind(CommandBuffer* p_CommandBuffer) const = 0;
			virtual void Unbind() const = 0;

			virtual uint32_t GetCount() const = 0;

			static Ref<IndexBuffer> Create(uint32_t* p_Indices, uint32_t p_Count);
	};

} // namespace KTN
