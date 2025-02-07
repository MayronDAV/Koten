#pragma once
#include "Koten/Core/Base.h"
#include "Renderpass.h"



namespace KTN
{
	class KTN_API Framebuffer
	{
		public:
			virtual ~Framebuffer() = default;

			virtual void Validate() {};

			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;

			static Ref<Framebuffer> Create(const FramebufferSpecification& p_Spec);

			static Ref<Framebuffer> Get(const FramebufferSpecification& p_Spec = {});
			static void ClearCache();
			static void DeleteUnusedCache();
	};
}