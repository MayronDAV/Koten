#pragma once
#include "Koten/Core/Base.h"
#include "Buffer.h"



namespace KTN
{
	class KTN_API VertexArray
	{
		public:
			virtual ~VertexArray() = default;

			virtual void Bind(CommandBuffer* p_CommandBuffer) const = 0;
			virtual void Unbind() const = 0;

			virtual void SetVertexBuffer(const Ref<VertexBuffer>& p_VertexBuffer) = 0;
			virtual void SetIndexBuffer(const Ref<IndexBuffer>& p_IndexBuffer) = 0;

			virtual const Ref<VertexBuffer>& GetVertexBuffer() const = 0;
			virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

			static Ref<VertexArray> Create();
	};

} // namespace KTN
