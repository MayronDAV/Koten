#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"
#include "VertexArray.h"
#include "DescriptorSet.h"



namespace KTN
{

	class KTN_API CommandBuffer
	{
		public:
			virtual ~CommandBuffer() = default;

			virtual bool Init() = 0;

			void BindSets(const Ref<DescriptorSet>* p_Sets, uint32_t p_Count = 1);

			virtual void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height) = 0;
			virtual void DispatchCompute(uint32_t p_NumGroups_X, uint32_t p_NumGroups_Y, uint32_t p_NumGroups_Z) = 0;
			
			// sets the depth bias for the current pipeline
			virtual void SetDepthBias(float p_ConstantFactor, float p_SlopeFactor) = 0;
			// sets the stencil for the current pipeline, p_Compare is just for opengl
			virtual void SetStencil(StencilFace p_Face, StencilCompare p_Compare, uint32_t p_CompareMask = 0xFF, uint32_t p_WriteMask = 0xFF, int p_Reference = 1) = 0;

			virtual void Draw(DrawType p_Type, const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount) = 0;
			virtual void DrawIndexed(DrawType p_Type, const Ref<VertexArray>& p_VertexArray) = 0;

			static Unique<CommandBuffer> Create();
	};
}