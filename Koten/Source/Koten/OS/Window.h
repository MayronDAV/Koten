#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Events/Event.h"
#include "Koten/Core/Definitions.h"
#include "Koten/Graphics/GraphicsContext.h"

// lib
#include <glm/glm.hpp>



namespace KTN
{
	class KTN_API Window
	{
		public:
			using EventCallbackFn = std::function<void(Event&)>;

		public:
			virtual ~Window() = default;

			virtual void SwapBuffer()											= 0;

			virtual void Maximize()												= 0;
			virtual void Minimize()												= 0;
			virtual void Restore()												= 0;

			virtual void SubmitEvent(Event& p_Event)							= 0;
			virtual void SetPosition(int p_X, int p_Y)							= 0;
			virtual void SetEventCallback(EventCallbackFn p_Callback)			= 0;
			virtual void SetCursorMode(CursorMode p_Mode)						= 0;
			virtual void SetVsync(bool p_Value)									= 0;

			virtual void Resize(uint32_t p_Width, uint32_t p_Height)			= 0;
			virtual void ChangeMode(WindowMode p_Mode, bool p_Maximize = true)	= 0;

			virtual bool IsMaximized() const									= 0;
			virtual bool IsMinimized() const									= 0;

			virtual Unique<GraphicsContext>& GetContext()						= 0;

			virtual glm::vec2 GetPosition() const								= 0;

			virtual std::vector<WindowResolution> GetResolutions() const		= 0;

			virtual uint32_t GetWidth() const									= 0;
			virtual uint32_t GetHeight() const									= 0;
			virtual void* GetNative()											= 0;
			virtual bool IsVsync() const										= 0;

			static Unique<Window> Create(const WindowSpecification& p_Spec = {});
	};


} // namespace KTN