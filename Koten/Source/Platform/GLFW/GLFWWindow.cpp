#include "ktnpch.h"
#include "GLFWWindow.h"

#include "Koten/Events/Event.h"
#include "Koten/Events/KeyEvent.h"
#include "Koten/Events/MouseEvent.h"
#include "Koten/Events/WindowEvent.h"

// lib
#include <glad/glad.h>



namespace KTN
{
	namespace
	{
		static void GLFWErrorCallback(int p_Error, const char* p_Description)
		{
			KTN_GLFW_ERROR("GLFW Error ({0}): {1}", p_Error, p_Description)
		}

		static uint8_t s_GLFWWindowCount = 0;

	} // namespace

	GLFWWindow::GLFWWindow(const WindowSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION_LOW();

		Init(p_Spec);
	}

	GLFWWindow::~GLFWWindow()
	{
		KTN_PROFILE_FUNCTION_LOW();

		Shutdown();
	}

	void GLFWWindow::SwapBuffer()
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_Data.Context->SwapBuffer();
	}

	void GLFWWindow::OnUpdate()
	{
		KTN_PROFILE_FUNCTION_LOW();

		glfwPollEvents();
	}

	void GLFWWindow::Maximize()
	{
		KTN_PROFILE_FUNCTION_LOW();

		glfwMaximizeWindow(m_Window);
	}

	void GLFWWindow::Minimize()
	{
		KTN_PROFILE_FUNCTION_LOW();

		glfwIconifyWindow(m_Window);
	}

	void GLFWWindow::Restore()
	{
		KTN_PROFILE_FUNCTION_LOW();

		glfwRestoreWindow(m_Window);
	}

	void GLFWWindow::SubmitEvent(Event& p_Event)
	{
		KTN_PROFILE_FUNCTION_LOW();

		if (m_Data.EventCallback)
			m_Data.EventCallback(p_Event);
	}

	void GLFWWindow::SetPosition(int p_X, int p_Y)
	{
		KTN_PROFILE_FUNCTION_LOW();

		glfwSetWindowPos(m_Window, p_X, p_Y);
	}

	void GLFWWindow::SetVsync(bool p_Value)
	{
		KTN_PROFILE_FUNCTION_LOW();

		m_Data.Vsync = p_Value;
		m_Data.Context->SetVsync(p_Value);
	}

	void GLFWWindow::Resize(uint32_t p_Width, uint32_t p_Height)
	{
		KTN_PROFILE_FUNCTION_LOW();

		if (m_Data.Mode != WindowMode::Windowed)
			return;

		glfwSetWindowSize(m_Window, (int)p_Width, (int)p_Height);
	}

	void GLFWWindow::ChangeMode(WindowMode p_Mode, bool p_Maximize)
	{
		KTN_PROFILE_FUNCTION_LOW();

		int posX = 0, posY = 0;
		glfwGetWindowPos(m_Window, &posX, &posY);

		glfwHideWindow(m_Window);

		int width				= int(m_Data.Width);
		int height				= int(m_Data.Height);
		GLFWmonitor* monitor	= nullptr;

		if (p_Mode == WindowMode::Fullscreen)
			monitor = glfwGetPrimaryMonitor();

		const GLFWvidmode* mode = glfwGetVideoMode(monitor ? monitor : glfwGetPrimaryMonitor());

		if (p_Maximize)
		{
			width = mode->width;
			height = mode->height;
		}

		glfwSetWindowMonitor(m_Window, monitor, posX, posY, width, height, GLFW_DONT_CARE);

		glfwSetWindowAttrib(m_Window, GLFW_DECORATED, p_Mode != WindowMode::Borderless);

		if (p_Mode != WindowMode::Fullscreen && p_Maximize)
		{
			Maximize();
		}

		glfwShowWindow(m_Window);

		m_Data.Width		= width;
		m_Data.Height		= height;
		m_Data.Mode			= p_Mode;
	}

	bool GLFWWindow::IsMaximized() const
	{
		return glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED) == GLFW_TRUE;
	}

	bool GLFWWindow::IsMinimized() const
	{
		return glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED) == GLFW_TRUE;
	}

	glm::vec2 GLFWWindow::GetPosition() const
	{
		KTN_PROFILE_FUNCTION_LOW();

		int px, py;
		glfwGetWindowPos(m_Window, &px, &py);

		return glm::vec2(px, py);
	}

	std::vector<WindowResolution> GLFWWindow::GetResolutions() const
	{
		KTN_PROFILE_FUNCTION_LOW();

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		int count = 0;
		const GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);

		std::vector<WindowResolution> resolutions;

		for (int i = 0; i < count; i++)
		{
			WindowResolution& res = resolutions.emplace_back();
			res.Width = modes[i].width;
			res.Height = modes[i].height;
			res.RefreshRate = modes[i].refreshRate;
		}

		return resolutions;
	}


	void GLFWWindow::Init(const WindowSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION_LOW();

		#define SET_EVENT(event) 			\
		if (data.EventCallback)				\
			data.EventCallback(event);		\

		m_Data.Title		= p_Spec.Title;
		m_Data.Width		= p_Spec.Width;
		m_Data.Height		= p_Spec.Height;
		m_Data.Mode			= p_Spec.Mode;
		m_Data.Resizable	= p_Spec.Resizable;
		m_Data.Vsync		= p_Spec.Vsync;

		KTN_GLFW_INFO("Creating window {0} ({1}, {2}) Vsync: {3}", p_Spec.Title, p_Spec.Width, p_Spec.Height, p_Spec.Vsync ? "true" : "false");

		if (s_GLFWWindowCount == 0)
		{
			int status = glfwInit();
			KTN_CORE_ASSERT(status, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		{
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			float xscale, yscale;
			glfwGetMonitorContentScale(monitor, &xscale, &yscale);
			m_Data.DPIScale = xscale;
		}

		{
			glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

			m_Data.Context = GraphicsContext::Create();

			int width = int(p_Spec.Width);
			int height = int(p_Spec.Height);
			GLFWmonitor* monitor = nullptr;

			if (p_Spec.Mode == WindowMode::Fullscreen)
				monitor = glfwGetPrimaryMonitor();

			glfwWindowHint(GLFW_DECORATED, p_Spec.Mode != WindowMode::Borderless);

			if (p_Spec.Maximize || p_Spec.Mode == WindowMode::Borderless)
			{
				auto mode = glfwGetVideoMode(monitor ? monitor : glfwGetPrimaryMonitor());
				width = mode->width;
				height = mode->height;
			}

			if (p_Spec.Mode == WindowMode::Windowed)
				glfwWindowHint(GLFW_MAXIMIZED, p_Spec.Maximize);

			glfwWindowHint(GLFW_RESIZABLE, p_Spec.Resizable);

			m_Window = glfwCreateWindow((int)p_Spec.Width, (int)p_Spec.Height, p_Spec.Title.c_str(), nullptr, nullptr);
			KTN_CORE_ASSERT(m_Window, "Failed to create window!");

			if (p_Spec.Center && p_Spec.Mode == WindowMode::Windowed && !p_Spec.Maximize)
			{
				auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

				int posX = (mode->width - p_Spec.Width) / 2;
				int posY = (mode->height - p_Spec.Height) / 2;

				glfwSetWindowPos(m_Window, posX, posY);
			}

			glfwShowWindow(m_Window);
		}
		++s_GLFWWindowCount;
		m_Data.Context->Init(m_Window, m_Data.Title.c_str());

		glfwSetWindowUserPointer(m_Window, &m_Data);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* p_Window, int p_Width, int p_Height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);
			data.Width = p_Width;
			data.Height = p_Height;

			WindowResizeEvent event(p_Width, p_Height);
			SET_EVENT(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* p_Window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);
			WindowCloseEvent event;
			SET_EVENT(event);
		});

		glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* p_Window, int p_Focused)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);
			if (p_Focused)
			{
				WindowFocusEvent event;
				SET_EVENT(event);
			}
			else
			{
				WindowLostFocusEvent event;
				SET_EVENT(event);
			}
		});

		glfwSetWindowPosCallback(m_Window, [](GLFWwindow* p_Window, int p_PosX, int p_PosY)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);
			WindowMovedEvent event(p_PosX, p_PosY);
			SET_EVENT(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* p_Window, int p_Key, int p_Scancode, int p_Action, int p_Mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

			switch (p_Action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(p_Key, 0);
					SET_EVENT(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(p_Key);
					SET_EVENT(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(p_Key, true);
					SET_EVENT(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* p_Window, unsigned int p_Keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

			KeyTypedEvent event(p_Keycode);
			SET_EVENT(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* p_Window, int p_Button, int p_Action, int p_Mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

			switch (p_Action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(p_Button);
					SET_EVENT(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(p_Button);
					SET_EVENT(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* p_Window, double p_OffsetX, double p_OffsetY)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

			MouseScrolledEvent event((float)p_OffsetX, (float)p_OffsetY);
			SET_EVENT(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* p_Window, double p_X, double p_Y)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

			MouseMovedEvent event((float)p_X, (float)p_Y);
			SET_EVENT(event);
		});

		#undef SET_EVENT
	}

	void GLFWWindow::Shutdown() noexcept
	{
		KTN_PROFILE_FUNCTION_LOW();

		KTN_GLFW_WARN("{} window shutdown", m_Data.Title)
		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
		{
			glfwTerminate();
		}
	}

} // namespace KTN