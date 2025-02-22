#pragma once
#include "Koten/Core/Base.h"
#include "CommandBuffer.h"

// lib
#include <glm/glm.hpp>



namespace KTN
{
	struct Capabilities
	{
		int MaxSamples						= 4;
		float MaxAnisotropy					= 4.0f;
		int MaxTextureUnits					= 32;
		float MaxLineWidth					= 1.0f;
		bool WideLines						= true;
		bool SupportCompute					= true;
		bool FillModeNonSolid				= true;
		bool SupportTesselation				= true;
		bool SupportGeometry				= true;
		bool SamplerAnisotropy				= true;
	};

	class KTN_API RendererAPI
	{
		public:
			virtual ~RendererAPI() = default;

			virtual void Begin() = 0;
			virtual void End() = 0;

			virtual void ClearColor(const glm::vec4& p_Color) = 0;
			virtual void ClearRenderTarget(const Ref<Texture2D>& p_Texture, uint32_t p_Value) = 0;
			virtual void ClearRenderTarget(const Ref<Texture2D>& p_Texture, const glm::vec4& p_Value) = 0;

			virtual void OnResize(uint32_t p_Width, uint32_t p_Height) {}

			virtual const Capabilities& GetCapabilities() const = 0;

			virtual CommandBuffer* GetCurrentCommandBuffer() = 0;

			static RendererAPI* Create();
	};


} // namespace KTN