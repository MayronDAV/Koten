#include "ktnpch.h"
#include "ImGuiLayer.h"

#include "Platform/OpenGL/GLImGuiLayer.h"

// lib
#include <GLFW/glfw3.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>



namespace KTN
{
	Ref<ImGuiLayer> ImGuiLayer::Create()
	{
		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLImGuiLayer>();

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	void ImGuiLayer::Init()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.IniFilename = "Resources/Layout.ini";
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;		// Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Enable Multi-Viewport / Platform Windows
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

	}

	void ImGuiLayer::NewFrame()
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::EndFrame()
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = nullptr;
			if (Engine::GetAPI() == RenderAPI::OpenGL)
				backup_current_context = glfwGetCurrentContext();

			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();

			if (Engine::GetAPI() == RenderAPI::OpenGL && backup_current_context)
				glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::Shutdown()
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& p_Event)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io		 = ImGui::GetIO();
			p_Event.Handled |= p_Event.IsInCategory(EventCategoryMouse)		& io.WantCaptureMouse;
			p_Event.Handled |= p_Event.IsInCategory(EventCategoryKeyboard)	& io.WantCaptureKeyboard;
			p_Event.Handled |= p_Event.IsInCategory(EventCategoryKeyboard)	& io.WantTextInput;
		}
	}

	ImGuiContext* ImGuiLayer::GetCurrentContext()
	{
		return ImGui::GetCurrentContext();
	}

	uint32_t ImGuiLayer::GetActiveWidgetID() const
	{
		KTN_CORE_ASSERT(GImGui)

		return GImGui->ActiveId;
	}

}
