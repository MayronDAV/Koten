#pragma once
#include "Koten/OS/Window.h"
#include "GLFWBase.h"



namespace KTN
{
	class GLFWWindow : public Window
	{
		public:
			GLFWWindow(const WindowSpecification& p_Spec = {});
			~GLFWWindow() override;

			void SwapBuffer() override;
			void OnUpdate() override;

			void Maximize() override;
			void Minimize() override;
			void Restore() override;

			float GetDPIScale() const { return m_Data.DPIScale; }

			void SubmitEvent(Event& p_Event) override;
			void SetEventCallback(EventCallbackFn p_Callback) override { m_Data.EventCallback = p_Callback; }
			void SetPosition(int p_X, int p_Y) override;
			void SetVsync(bool p_Value) override;

			void Resize(uint32_t p_Width, uint32_t p_Height) override;
			void ChangeMode(WindowMode p_Mode, bool p_Maximize = true) override;

			bool IsMaximized() const override;
			bool IsMinimized() const override;

			Unique<GraphicsContext>& GetContext() override { return m_Data.Context; }

			glm::vec2 GetPosition() const override;

			std::vector<WindowResolution> GetResolutions() const override;

			uint32_t GetWidth() const override { return m_Data.Width; }
			uint32_t GetHeight() const override { return m_Data.Height; }
			void* GetNative() override { return m_Window; }
			bool IsVsync() const override { return m_Data.Vsync; }

		private:
			void Init(const WindowSpecification& p_Spec);
			void Shutdown() noexcept;
			void UpdateControllers();

		private:
			GLFWwindow* m_Window				= nullptr;

			struct WindowData
			{
				std::string Title				= "Koten";
				uint32_t Width					= 800;
				uint32_t Height					= 600;
				WindowMode Mode					= WindowMode::Windowed;
				bool Resizable					= true;
				bool Vsync						= false;
				float DPIScale					= 1.0f;

				EventCallbackFn EventCallback	= nullptr;
				Unique<GraphicsContext> Context = nullptr;
			} m_Data;
	};


} // namespace KTN