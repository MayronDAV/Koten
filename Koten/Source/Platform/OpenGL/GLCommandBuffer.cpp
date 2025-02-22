#include "ktnpch.h"
#include "GLBase.h"
#include "GLUtils.h"
#include "GLCommandBuffer.h"
#include "GLIndirectBuffer.h"



namespace KTN
{
	void GLCommandBuffer::SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height)
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glViewport((GLint)p_X, (GLint)p_Y, (GLsizei)p_Width, (GLsizei)p_Height));
	}

	void GLCommandBuffer::DispatchCompute(uint32_t p_NumGroups_X, uint32_t p_NumGroups_Y, uint32_t p_NumGroups_Z)
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glDispatchCompute(p_NumGroups_X, p_NumGroups_Y, p_NumGroups_Z));
	}

	void GLCommandBuffer::SetDepthBias(float p_ConstantFactor, float p_SlopeFactor)
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glPolygonOffset(p_ConstantFactor, p_SlopeFactor));
	}

	void GLCommandBuffer::SetStencil(StencilFace p_Face, StencilCompare p_Compare, uint32_t p_CompareMask, uint32_t p_WriteMask, int p_Reference)
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLCall(glStencilFuncSeparate(
			GLUtils::StencilFaceToGL(p_Face),
			GLUtils::StencilCompareToGL(p_Compare),
			p_Reference,
			p_CompareMask));
		GLCall(glStencilMaskSeparate(GLUtils::StencilFaceToGL(p_Face), p_WriteMask));
	}

	void GLCommandBuffer::Draw(DrawType p_Type, const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount)
	{
		KTN_PROFILE_FUNCTION_LOW();

		KTN_CORE_VERIFY(p_VertexCount > 0);
		if (p_VertexArray)
			p_VertexArray->Bind(this);

		glDrawArrays(GLUtils::DrawTypeToGL(p_Type), 0, p_VertexCount);
	}

	void GLCommandBuffer::DrawIndexed(DrawType p_Type, const Ref<VertexArray>& p_VertexArray)
	{
		KTN_PROFILE_FUNCTION_LOW();

		KTN_CORE_VERIFY(p_VertexArray != nullptr);
		KTN_CORE_VERIFY(p_VertexArray->GetIndexBuffer());
		KTN_CORE_VERIFY(p_VertexArray->GetIndexBuffer()->GetCount() > 0);

		p_VertexArray->Bind(this);
		uint32_t count = p_VertexArray->GetIndexBuffer()->GetCount();

		glDrawElements(GLUtils::DrawTypeToGL(p_Type), count, GL_UNSIGNED_INT, nullptr);
	}

	void GLCommandBuffer::DrawIndexedIndirect(DrawType p_Type, const Ref<VertexArray>& p_VertexArray, const Ref<IndirectBuffer>& p_Buffer)
	{
		KTN_CORE_VERIFY(p_VertexArray != nullptr);
		KTN_CORE_VERIFY(p_VertexArray->GetIndexBuffer());
		KTN_CORE_VERIFY(p_VertexArray->GetIndexBuffer()->GetCount() > 0);

		p_VertexArray->Bind(this);

		As<IndirectBuffer, GLIndirectBuffer>(p_Buffer)->Bind();

		GLCall(glDrawElementsIndirect(GLUtils::DrawTypeToGL(p_Type), GL_UNSIGNED_INT, nullptr));
	}

} // namespace KTN
