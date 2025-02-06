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

	void GLCommandBuffer::Draw(const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount)
	{
		KTN_CORE_VERIFY(p_VertexCount > 0);
		p_VertexArray->Bind(this);

		// TODO: a way to set different draw modes!
		glDrawArrays(GL_TRIANGLES, 0, p_VertexCount);
	}

	void GLCommandBuffer::DrawIndexed(const Ref<VertexArray>& p_VertexArray)
	{
		KTN_CORE_VERIFY(p_VertexArray->GetIndexBuffer());
		KTN_CORE_VERIFY(p_VertexArray->GetIndexBuffer()->GetCount() > 0);

		p_VertexArray->Bind(this);
		uint32_t count = p_VertexArray->GetIndexBuffer()->GetCount();

		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

} // namespace KTN
