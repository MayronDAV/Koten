#include "Koten/Koten.h"
#include "Koten/EntryPoint.h"

// lib
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace KTN
{
	class SandboxLayer : public Layer
	{
		private:
			Ref<Shader> m_Shader = nullptr;
			Ref<VertexArray> m_VAO = nullptr;
			Ref<Texture2D> m_CheckerTexture = nullptr;
			Ref<Texture2D> m_WallTexture = nullptr;
			Ref<DescriptorSet> m_Set = nullptr;
			Camera m_Camera;
			float m_Distance = 5.0f;
			float m_Zoom = 1.0f;
			float m_Speed = 4.0f;
			bool m_Orthographic = false;
			glm::vec2 m_TileSize = { 0.90f, 0.90f };
			glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };

		public:
			SandboxLayer() : Layer("SandboxLayer") {}
			~SandboxLayer() = default;

			void OnAttach() override 
			{
				KTN_INFO("Attaching...");

				m_CheckerTexture = TextureImporter::LoadTexture2D("Assets/Textures/checkerboard.png");
				m_WallTexture = TextureImporter::LoadTexture2D("Assets/Textures/wall.jpg");

				m_Shader = Shader::Create("Assets/Shaders/ShaderTest.glsl");

				m_Set = DescriptorSet::Create({ 0, m_Shader });

				m_VAO = VertexArray::Create();


				float vertices[] = {
					// positions      // texcoords  // colors
					-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
					 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
					 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
					-0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f
				};

				auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
				vbo->SetLayout({
					{ DataType::Float3 , "a_Position"	},
					{ DataType::Float2 , "a_Texcoord"	},
					{ DataType::Float3 , "a_Color"		},
				});
				m_VAO->SetVertexBuffer(vbo);

				uint32_t indices[] = {
					0, 1, 3, // first triangle
					1, 2, 3  // second triangle
				};
				auto ebo = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
				m_VAO->SetIndexBuffer(ebo);

			}
			void OnDetach() override { KTN_INFO("Detaching..."); }
			void OnUpdate() override
			{
				auto& window = Application::Get().GetWindow();
				m_Camera.SetViewportSize(window->GetWidth(), window->GetHeight());
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
				auto commandBuffer = RendererCommand::GetCurrentCommandBuffer();

				auto& window = Application::Get().GetWindow();
				commandBuffer->SetViewport(0.0f, 0.0f, window->GetWidth(), window->GetHeight());

				m_Shader->Bind();

				m_Set->SetTexture("u_Texture", m_WallTexture);
				m_Set->Upload(commandBuffer);

				commandBuffer->BindSets(&m_Set);

				glm::mat4 projView = m_Camera.GetProjection() * glm::inverse(glm::translate(glm::mat4(1.0f), { m_Position.x, m_Position.y, m_Distance }));

				int size = 5;
				for (int y = 0; y < size; y++)
				{
					for (int x = 0; x < size; x++)
					{
						glm::mat4 model = glm::translate(glm::mat4(1.0f), { x, y, 0.0f }) * glm::scale(glm::mat4(1.0f), { m_TileSize, 1.0f });
						glm::mat4 matrix = projView * model;

						m_Shader->SetPushValue("Matrix", &matrix);
						m_Shader->BindPushConstants(commandBuffer);
						RendererCommand::DrawIndexed(m_VAO);
					}
				}

				m_Set->SetTexture("u_Texture", m_CheckerTexture);
				m_Set->Upload(commandBuffer);

				commandBuffer->BindSets(&m_Set);

				glm::mat4 model = glm::translate(glm::mat4(1.0f), m_Position);
				glm::mat4 matrix = projView * model;

				m_Shader->SetPushValue("Matrix", &matrix);
				m_Shader->BindPushConstants(commandBuffer);
				RendererCommand::DrawIndexed(m_VAO);
			}
			void OnImgui() override
			{
				ImGui::Begin("Editor");
				ImGui::DragFloat("Distance", &m_Distance, 0.1f);
				ImGui::DragFloat2("Tile Size", glm::value_ptr(m_TileSize), 0.1f);
				ImGui::DragFloat("Zoom", &m_Zoom, 0.01f, 0.01f, 10.0f);
				ImGui::DragFloat("Speed", &m_Speed, 0.01f, 0.01f);
				if (ImGui::RadioButton("Orthographic", m_Orthographic))
					m_Orthographic = !m_Orthographic;
				ImGui::End();
			}
			void OnEvent(Event& p_Event) override {}
	};



	Application* CreateApplication(int p_Argc, char** p_Argv)
	{
		KTN_INFO("Creating sandbox app...");

		auto app = new Application();
		ImGui::SetCurrentContext(app->GetImGui()->GetCurrentContext());

		app->PushLayer(CreateRef<SandboxLayer>());

		return app;
	}

} // namespace KTN