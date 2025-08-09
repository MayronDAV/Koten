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
#include "Koten/Script/ScriptEngine.h"
#include "Koten/Scene/SceneManager.h"




namespace KTN
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationConfig& p_Config)
	{
		KTN_PROFILE_FUNCTION();

		s_Instance = this;

		Engine::Init();
		
		Engine::Get().LoadSettings();

		auto& settings = Engine::Get().GetSettings();

		m_UpdateMinimized = settings.UpdateMinimized;

		WindowSpecification spec = {};
		spec.Title = p_Config.Title;
		spec.Width = settings.Width;
		spec.Height = settings.Height;
		spec.Mode = settings.Mode;
		spec.Resizable = settings.Resizable;
		spec.Maximize = settings.Maximize;
		spec.Center = settings.Center;
		spec.Vsync = settings.Vsync;
		spec.IconPath = p_Config.IconPath;
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
		ScriptEngine::Init();
	}

	Application::~Application()
	{
		KTN_PROFILE_FUNCTION();

		m_LayerStack.Clear();

		SceneManager::Shutdown();

		Renderer::Shutdown();

		Pipeline::ClearCache();
		Framebuffer::ClearCache();
		Renderpass::ClearCache();
		Texture::ClearCache();

		KTN_PROFILE_SHUTDOWN();

		ScriptEngine::Shutdown();
		RendererCommand::Release();

		Engine::Shutdown();

		s_Instance = nullptr;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			KTN_PROFILE_FRAME("MainThread");

			ExecuteMainThreadQueue();

			// Update
			if (( m_Window->IsMinimized() && m_UpdateMinimized ) || !m_Window->IsMinimized())
			{
				KTN_PROFILE_SCOPE("Update");

				Time::OnUpdate();

				Engine::Get().ResetStats();

				if (auto currentTime = Time::GetTime();
					currentTime - m_LastTime >= 1.0)
				{
					Engine::Get().GetStats().FramesPerSecond = uint32_t(m_Counter / (currentTime - m_LastTime));
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
				KTN_PROFILE_SCOPE("Render");

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

	void Application::SubmitToMainThread(const std::function<void()>& p_Func)
	{
		KTN_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
		
		m_MainThreadQueue.emplace_back(p_Func);
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

	void Application::ExecuteMainThreadQueue()
	{
		KTN_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		for (auto& func : m_MainThreadQueue)
			func();

		m_MainThreadQueue.clear();
	}

} // namespace KTN
