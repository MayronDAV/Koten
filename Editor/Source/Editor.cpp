#include "Editor.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



namespace KTN
{
	namespace
	{
		void BeginDockspace(bool p_MenuBar)
		{
			KTN_PROFILE_FUNCTION();

			static bool opt_fullscreen = true;
			static bool opt_padding = false;
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
			if (p_MenuBar)
				window_flags |= ImGuiWindowFlags_MenuBar;

			if (opt_fullscreen)
			{
				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->WorkPos);
				ImGui::SetNextWindowSize(viewport->WorkSize);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			}
			else
			{
				dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
			}

			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 1.0f });
			if (!opt_padding)
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("SandboxDockspace", nullptr, window_flags);
			if (!opt_padding)
				ImGui::PopStyleVar();
			ImGui::PopStyleColor(); // windowBg

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// Submit the DockSpace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}
		}

		void EndDockspace()
		{
			KTN_PROFILE_FUNCTION();

			ImGui::End();
		}

	} // namespace

	Editor::Editor()
		: Layer("Editor")
	{
	}

	Editor::~Editor()
	{
	}

	void Editor::OnAttach()
	{
		KTN_PROFILE_FUNCTION();

		m_ActiveScene = CreateRef<Scene>();

		{
			auto entt = m_ActiveScene->CreateEntity("Camera");
			entt.AddComponent<TransformComponent>(glm::vec3(0.0f, 0.0f, 5.0f));
			entt.AddComponent<CameraComponent>();
		}

		for (int x = 0; x < 5; x++)
		{
			for (int y = 0; y < 5; y++)
			{
				auto square = m_ActiveScene->CreateEntity("Square");
				square.AddComponent<TransformComponent>(glm::vec3(x, y, 0.0f), glm::vec3(0.9f, 0.9f, 1.0f));
				square.AddComponent<SpriteComponent>(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
			}
		}
	}

	void Editor::OnDetach()
	{
	}

	void Editor::OnUpdate()
	{
		KTN_PROFILE_FUNCTION();

		TextureSpecification tspec = {};
		tspec.Width = m_Width;
		tspec.Height = m_Height;
		tspec.Format = TextureFormat::RGBA32_FLOAT;
		tspec.Usage = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
		tspec.Samples = 1;
		tspec.GenerateMips = false;
		tspec.AnisotropyEnable = false;
		tspec.DebugName = "MainTexture";

		m_MainTexture = Texture2D::Get(tspec);

		m_ActiveScene->SetRenderTarget(m_MainTexture);
		m_ActiveScene->SetViewportSize(m_Width, m_Height);
		m_ActiveScene->OnUpdate();

		Renderer::Clear();
	}

	void Editor::OnRender()
	{
		KTN_PROFILE_FUNCTION();

		m_ActiveScene->OnRender();
	}

	void Editor::OnImgui()
	{
		KTN_PROFILE_FUNCTION();

		BeginDockspace(false);

		ImGui::SetNextWindowSizeConstraints({ 400.0f, 400.0f }, { FLT_MAX, FLT_MAX });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 1.0f });
		ImGui::Begin("Viewport");
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		{
			ImVec2 viewportSize = ImGui::GetContentRegionAvail();

			UI::Image(m_MainTexture, { (float)m_Width, (float)m_Height });

			m_Width = (uint32_t)std::max(viewportSize.x, 400.0f);
			m_Height = (uint32_t)std::max(viewportSize.y, 400.0f);
		}
		ImGui::End();

		ImGui::Begin("Info");
		{
			auto& stats = Engine::GetStats();

			ImGui::Text("FPS: %i", (int)stats.FramesPerSecond);
			ImGui::Text("DrawCalls: %u", stats.DrawCalls);
			ImGui::Text("TrianglesCount: %u", stats.TrianglesCount);
		}
		ImGui::End();

		EndDockspace();
	}

	void Editor::OnEvent(Event& p_Event)
	{
	}

} // namespace KTN