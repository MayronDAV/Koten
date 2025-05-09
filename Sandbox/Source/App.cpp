#include "Koten/Koten.h"
#include "Koten/EntryPoint.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



namespace KTN
{
	class SandboxLayer : public Layer
	{
		private:
			Ref<Texture2D> m_MainTexture = nullptr;
			Ref<Texture2D> m_CheckerTexture = nullptr;
			Ref<Texture2D> m_WallTexture = nullptr;
			Camera m_Camera;
			float m_Distance = 5.0f;
			float m_Zoom = 1.0f;
			float m_Speed = 4.0f;
			bool m_Orthographic = false;
			int m_MapSize = 5;
			glm::vec2 m_TileSize = { 0.90f, 0.90f };
			glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
			uint32_t m_Width = 800;
			uint32_t m_Height = 600;

		public:
			SandboxLayer() : Layer("SandboxLayer") {}
			~SandboxLayer() = default;

			void OnAttach() override 
			{
				KTN_PROFILE_FUNCTION();

				KTN_INFO("Attaching...");

				m_CheckerTexture = TextureImporter::LoadTexture2D("Assets/Textures/checkerboard.png");
				m_WallTexture = TextureImporter::LoadTexture2D("Assets/Textures/wall.jpg");
			}
			void OnDetach() override { KTN_INFO("Detaching..."); }
			void OnUpdate() override
			{
				KTN_PROFILE_FUNCTION();

				m_Camera.SetViewportSize(m_Width, m_Height);
				m_Camera.SetZoom(m_Zoom);
				m_Camera.SetIsOrthographic(m_Orthographic);
				if (m_Orthographic)
				{
					m_Camera.SetFar(1.0f);
					m_Camera.SetNear(-1.0f);
				}
				else
				{
					m_Camera.SetFar(1000.0f);
					m_Camera.SetNear(0.001f);
				}

				m_Camera.OnUpdate();

				glm::vec3 dir{ 0.0f };

				if (Input::IsKeyPressed(Key::W))
					dir.y =  1;
				if (Input::IsKeyPressed(Key::A))
					dir.x = -1;
				if (Input::IsKeyPressed(Key::S))
					dir.y = -1;
				if (Input::IsKeyPressed(Key::D))
					dir.x =  1;

				m_Position += (glm::length(dir) > 0 ? glm::normalize(dir) : dir) * m_Speed * (float)Time::GetDeltaTime();
			}
			void OnRender() override
			{
				KTN_PROFILE_FUNCTION();

				TextureSpecification tspec		= {};
				tspec.Width						= m_Width;
				tspec.Height					= m_Height;
				tspec.Format					= TextureFormat::RGBA32_FLOAT;
				tspec.Usage						= TextureUsage::TEXTURE_COLOR_ATTACHMENT;
				tspec.Samples					= 1;
				tspec.GenerateMips				= false;
				tspec.AnisotropyEnable			= false;
				tspec.DebugName					= "MainTexture";

				m_MainTexture					= Texture2D::Get(tspec);

				RenderBeginInfo info			= {};
				info.RenderTarget				= m_MainTexture;
				info.Width						= m_Width;
				info.Height						= m_Height;
				info.Projection					= m_Camera.GetProjection();
				info.View						= glm::inverse(glm::translate(glm::mat4(1.0f), { m_Position.x, m_Position.y, m_Distance }));

				Renderer::Begin(info);

				RenderCommand command			= {};

				for (int y = 0; y < m_MapSize; y++)
				{
					for (int x = 0; x < m_MapSize; x++)
					{
						glm::mat4 model			= glm::translate(glm::mat4(1.0f), { x, y, 0.0f }) * glm::scale(glm::mat4(1.0f), { m_TileSize, 1.0f });
						command.Transform		= model;
						command.Render2D.Texture = m_WallTexture;

						Renderer::Submit(command);
					}
				}

				glm::mat4 model					= glm::translate(glm::mat4(1.0f), m_Position);
				command.Transform				= model;
				command.Render2D.Texture		= m_CheckerTexture;

				Renderer::Submit(command);

				Renderer::End();
			}

			void OnImgui() override
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
					ImVec2 viewportSize	= ImGui::GetContentRegionAvail();

					UI::Image(m_MainTexture, { (float)m_Width, (float)m_Height });

					m_Width				= (uint32_t)std::max(viewportSize.x, 400.0f);
					m_Height			= (uint32_t)std::max(viewportSize.y, 400.0f);
				}
				ImGui::End();

				ImGui::Begin("Editor");
				{
					ImGui::DragFloat("Distance", &m_Distance, 0.1f);
					ImGui::DragInt("Map Size", &m_MapSize, 1, 1);
					ImGui::DragFloat2("Tile Size", glm::value_ptr(m_TileSize), 0.1f);
					ImGui::DragFloat("Zoom", &m_Zoom, 0.01f, 0.01f, 10.0f);
					ImGui::DragFloat("Speed", &m_Speed, 0.01f, 0.01f);
					if (ImGui::RadioButton("Orthographic", m_Orthographic))
						m_Orthographic = !m_Orthographic;
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
			void OnEvent(Event& p_Event) override {}
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
	};



	Application* CreateApplication(int p_Argc, char** p_Argv)
	{
		KTN_PROFILE_FUNCTION();

		auto app = new Application();
		ImGui::SetCurrentContext(app->GetImGui()->GetCurrentContext());

		app->PushLayer(CreateRef<SandboxLayer>());

		return app;
	}

} // namespace KTN