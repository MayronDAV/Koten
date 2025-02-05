#include "ktnpch.h"
#include "GLImGuiLayer.h"
#include "Koten/Core/Application.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>



namespace KTN
{
	void GLImGuiLayer::OnAttach()
	{
		Init();

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetNative());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460 core");
	}

	void GLImGuiLayer::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		Shutdown();
	}

	void GLImGuiLayer::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		NewFrame();
	}

	void GLImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto& window = Application::Get().GetWindow();
		io.DisplaySize = ImVec2((float)window->GetWidth(), (float)window->GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		EndFrame();
	}

} // namespace KTN
