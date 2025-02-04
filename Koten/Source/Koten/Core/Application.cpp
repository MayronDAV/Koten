#include "ktnpch.h"
#include "Application.h"
#include "Log.h"
#include "Koten/Events/Event.h"
#include "Koten/Events/ApplicationEvent.h"
#include "Koten/Events/WindowEvent.h"



namespace KTN
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;

		WindowSpecification spec = {};
		m_Window = Window::Create(spec);

		m_Window->SetEventCallback(
		[&] (Event& p_Event) 
		{
			OnEvent(p_Event);
		});
	}

	Application::~Application()
	{
		s_Instance = nullptr;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			// Update
			if (( m_Window->IsMinimized() && m_UpdateMinimized ) || !m_Window->IsMinimized())
			{
				for (auto& layer : m_LayerStack)
					layer->OnUpdate();
			}

			// Render
			if (!m_Window->IsMinimized())
			{
				for (auto& layer : m_LayerStack)
					layer->OnRender();

				m_Window->SwapBuffer();
			}
		}
	}

	void Application::PushLayer(const Ref<Layer>& p_Layer)
	{
		p_Layer->OnAttach();
		m_LayerStack.PushLayer(p_Layer);
	}

	void Application::PushOverlay(const Ref<Layer>& p_Overlay)
	{
		p_Overlay->OnAttach();
		m_LayerStack.PushOverlay(p_Overlay);
	}

	void Application::PopLayer(const Ref<Layer>& p_Layer)
	{
		p_Layer->OnDetach();
		m_LayerStack.PopLayer(p_Layer);
	}

	void Application::PopOverlay(const Ref<Layer>& p_Overlay)
	{
		p_Overlay->OnDetach();
		m_LayerStack.PopOverlay(p_Overlay);
	}

	void Application::OnEvent(Event& p_Event)
	{
		p_Event.Dispatch<WindowCloseEvent>(
		[&](WindowCloseEvent& p_Event)
		{
			m_Running = false;
			return true;
		});

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(p_Event);
			if (p_Event.Handled)
				break;
		}
	}

} // namespace KTN
