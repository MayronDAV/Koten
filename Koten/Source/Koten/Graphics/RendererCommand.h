#pragma once
#include "Koten/Core/Base.h"
#include "RendererAPI.h"


namespace KTN
{
	class KTN_API RendererCommand
	{
		public:
			static void Init();
			static void Release();

			static void Begin();
			static void End();

			static void OnResize(uint32_t p_Width, uint32_t p_Height);

			static void ClearColor(const glm::vec4& p_Color);

			static void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height);
			static void DispatchCompute(uint32_t p_NumGroups_X, uint32_t p_NumGroups_Y, uint32_t p_NumGroups_Z);

			static void Draw(DrawType p_Type, const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount);
			static void DrawIndexed(DrawType p_Type, const Ref<VertexArray>& p_VertexArray);

			static const Capabilities& GetCapabilities();
			static CommandBuffer* GetCurrentCommandBuffer();

		private:
			static RendererAPI* s_API;
	};


} // namespace KTN
