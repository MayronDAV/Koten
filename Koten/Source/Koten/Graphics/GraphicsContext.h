#pragma once
#include "Koten/Core/Base.h"



namespace KTN
{
	class KTN_API GraphicsContext
	{
		public:
			virtual ~GraphicsContext() = default;

			virtual void Init(void* p_Window, const char* p_Name) = 0;

			virtual void OnResize(uint32_t p_Width, uint32_t p_Height) {};

			virtual void SwapBuffer() = 0;

			virtual void SetVsync(bool p_Value) = 0;

			static Unique<GraphicsContext> Create();
	};

} // namespace LNR
