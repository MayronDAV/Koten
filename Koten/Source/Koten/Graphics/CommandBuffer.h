#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"
#include "VertexArray.h"



namespace KTN
{
	class KTN_API CommandBuffer
	{
		public:
			virtual ~CommandBuffer() = default;

			virtual bool Init() = 0;

			virtual void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height) = 0;
			virtual void DispatchCompute(uint32_t p_NumGroups_X, uint32_t p_NumGroups_Y, uint32_t p_NumGroups_Z) = 0;
			
			virtual void Draw(const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount) = 0;
			virtual void DrawIndexed(const Ref<VertexArray>& p_VertexArray) = 0;

			static Unique<CommandBuffer> Create();
	};
}