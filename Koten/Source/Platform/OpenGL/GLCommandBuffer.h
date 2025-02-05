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
	};
} // namespace KTN