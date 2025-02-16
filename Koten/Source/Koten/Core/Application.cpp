#include "ktnpch.h"
#include "Application.h"
#include "Log.h"
#include "Koten/Events/Event.h"
#include "Koten/Events/ApplicationEvent.h"
#include "Koten/Events/WindowEvent.h"
#include "Koten/Graphics/RendererCommand.h"
#include "Koten/Graphics/Texture.h"
#include "Koten/Graphics/Framebuffer.h"
#include "Koten/Graphics/Renderpass.h"
#include "Koten/Graphics/Pipeline.h"
#include "Koten/Graphics/Renderer.h"




namespace KTN
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		KTN_PROFILE_FUNCTION();

		s_Instance = this;

		WindowSpecification spec = {};
		m_Window = Window::Create(spec);

		m_Window->SetEventCallback(
		[&] (Event& p_Event) 
		{
			OnEvent(p_Event);
		});

		RendererCommand::Init();

		m_ImGui = ImGuiLayer::Create();
		PushOverlay(m_ImGui);

		Renderer::Init();
	}

	Application::~Application()
	{
		KTN_PROFILE_FUNCTION();

		Renderer::Shutdown();

		Pipeline::ClearCache();
		Framebuffer::ClearCache();
		Renderpass::ClearCache();
		Texture::ClearCache();

		KTN_PROFILE_SHUTDOWN();

		RendererCommand::Release();

		s_Instance = nullptr;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			KTN_PROFILE_FRAME("MainThread");

			// Update
			if (( m_Window->IsMinimized() && m_UpdateMinimized ) || !m_Window->IsMinimized())
			{
				KTN_PROFILE_SCOPE("On Update");

				Time::OnUpdate();

				Engine::ResetStats();

				if (auto currentTime = Time::GetTime();
					currentTime - m_LastTime >= 1.0)
				{
					Engine::GetStats().FramesPerSecond = uint32_t(m_Counter / (currentTime - m_LastTime));
					m_LastTime = currentTime;
					m_Counter = 0;
				}
				m_Counter++;

				for (auto& layer : m_LayerStack)
					layer->OnUpdate();
			}

			// Render
			if (!m_Window->IsMinimized())
			{
				KTN_PROFILE_SCOPE("On Render");

				RendererCommand::Begin();

				for (auto& layer : m_LayerStack)
					layer->OnRender();

				// ImGui
				m_ImGui->Begin();
				{
					for (auto& layer : m_LayerStack)
						layer->OnImgui();
				}
				m_ImGui->End();

				RendererCommand::End();

				m_Window->SwapBuffer();
			}

			m_Window->OnUpdate();

			if (!m_Window->IsMinimized())
			{
				KTN_PROFILE_SCOPE("Deleting Cache");

				Pipeline::DeleteUnusedCache();
				Framebuffer::DeleteUnusedCache();
				Renderpass::DeleteUnusedCache();
				Texture::DeleteUnusedCache();
			}
		}
	}

	void Application::PushLayer(const Ref<Layer>& p_Layer)
	{
		KTN_PROFILE_FUNCTION();

		p_Layer->OnAttach();
		m_LayerStack.PushLayer(p_Layer);
	}

	void Application::PushOverlay(const Ref<Layer>& p_Overlay)
	{
		KTN_PROFILE_FUNCTION();

		p_Overlay->OnAttach();
		m_LayerStack.PushOverlay(p_Overlay);
	}

	void Application::PopLayer(const Ref<Layer>& p_Layer)
	{
		KTN_PROFILE_FUNCTION();

		p_Layer->OnDetach();
		m_LayerStack.PopLayer(p_Layer);
	}

	void Application::PopOverlay(const Ref<Layer>& p_Overlay)
	{
		KTN_PROFILE_FUNCTION();

		p_Overlay->OnDetach();
		m_LayerStack.PopOverlay(p_Overlay);
	}

	void Application::OnEvent(Event& p_Event)
	{
		KTN_PROFILE_FUNCTION();

		p_Event.Dispatch<WindowCloseEvent>(
		[&](WindowCloseEvent& p_Event)
		{
			m_Running = false;
			return true;
		});

		p_Event.Dispatch<WindowResizeEvent>(
		[&](WindowResizeEvent& p_Event)
		{
			RendererCommand::OnResize(p_Event.GetHeight(), p_Event.GetHeight());
			return false;
		});

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			KTN_PROFILE_SCOPE("Layers OnEvent");

			(*--it)->OnEvent(p_Event);
			if (p_Event.Handled)
				break;
		}
	}

} // namespace KTN
