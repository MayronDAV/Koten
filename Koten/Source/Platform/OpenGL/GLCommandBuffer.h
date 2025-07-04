#pragma once
#include "Koten/Graphics/CommandBuffer.h"



namespace KTN
{
	class GLCommandBuffer : public CommandBuffer
	{
		public:
			GLCommandBuffer() = default;
			~GLCommandBuffer() override = default;

			bool Init() override { return true; }

			void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height) override;
			void DispatchCompute(uint32_t p_NumGroups_X, uint32_t p_NumGroups_Y, uint32_t p_NumGroups_Z) override;

			void EnableDepthBias(bool p_Enable) override;
			void EnableStencil(bool p_Enable) override;

			void SetDepthBias(float p_ConstantFactor, float p_SlopeFactor) override;
			void SetStencil(StencilFace p_Face, StencilCompare p_Compare, uint32_t p_CompareMask, uint32_t p_WriteMask, int p_Reference) override;

			void Draw(DrawType p_Type, const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount) override;
			void DrawIndirect(DrawType p_Type, const Ref<VertexArray>& p_VertexArray, const Ref<IndirectBuffer>& p_Buffer) override;
			void DrawIndexed(DrawType p_Type, const Ref<VertexArray>& p_VertexArray) override;
			void DrawIndexedIndirect(DrawType p_Type, const Ref<VertexArray>& p_VertexArray, const Ref<IndirectBuffer>& p_Buffer) override;
	};
} // namespace KTN