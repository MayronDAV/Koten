#pragma once
#include "Koten/Core/Base.h"
#include "Texture.h"


namespace KTN
{
	class KTN_API Renderpass
	{
		public:
			virtual ~Renderpass() = default;

			virtual void Begin(CommandBuffer* p_CommandBuffer, const Ref<Framebuffer>& p_Frame, uint32_t p_Width, uint32_t p_Height, const glm::vec4& p_Color = { 1, 1, 1, 1 }, SubpassContents p_Contents = SubpassContents::INLINE) = 0;
			virtual void End(CommandBuffer* p_CommandBuffer) = 0;

			virtual const RenderpassSpecification& GetSpecification() const = 0;
			virtual int GetAttachmentCount() const = 0;

			static Ref<Renderpass> Create(const RenderpassSpecification& p_Spec = {});

			static Ref<Renderpass> Get(const RenderpassSpecification& p_Spec = {});
			static void ClearCache();
			static void DeleteUnusedCache();
	};

} // namespace KTN