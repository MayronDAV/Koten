#include "ktnpch.h"
#include "Application.h"
#include "Log.h"
#include "Koten/Events/Event.h"
#include "Koten/Events/WindowEvent.h"



namespace KTN
{
	Application::Application()
	{
		WindowSpecification spec = {};
		m_Window = Window::Create(spec);

		m_Window->SetEventCallback(
		[&] (Event& p_Event) 
		{
			if (p_Event.GetEventType() == EventType::WindowClose)
				m_Running = false;
		});
	}

	Application::~Application()
	{
		if (m_Window)
			delete m_Window;
	}

	void Application::Run()
	{
		while (m_Running)
		{

			m_Window->OnUpdate();
		}
	}

} // namespace KTN
