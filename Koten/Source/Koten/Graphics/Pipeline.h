#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"
#include "Texture.h"
#include "CommandBuffer.h"
#include "Renderpass.h"
#include "Framebuffer.h"




namespace KTN
{
	class KTN_API Pipeline
	{
		public:
			virtual ~Pipeline() = default;

			virtual const Ref<Shader>& GetShader() = 0;

			virtual void Begin(CommandBuffer* p_CommandBuffer, SubpassContents p_Contents = SubpassContents::INLINE, int p_MipIndex = 0) = 0;
			virtual void End(CommandBuffer* p_CommandBuffer) = 0;

			uint32_t GetWidth();
			uint32_t GetHeight();

			virtual const Ref<Renderpass>& GetRenderpass() const = 0;
			virtual const Ref<Framebuffer>& GetFramebuffer(int p_MipIndex = 0) const = 0;

			static Ref<Pipeline> Create(const PipelineSpecification& p_Spec);

			static Ref<Pipeline> Get(const PipelineSpecification& p_Spec);
			static void ClearCache();
			static void DeleteUnusedCache();

		protected:
			PipelineSpecification m_Spec;
	};
}