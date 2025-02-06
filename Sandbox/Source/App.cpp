#include "Koten/Koten.h"
#include "Koten/EntryPoint.h"

// lib
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>


namespace KTN
{
	class SandboxLayer : public Layer
	{
		private:
			Ref<Shader> m_Shader = nullptr;
			Ref<VertexArray> m_VAO = nullptr;
			Ref<Texture2D> m_Texture = nullptr;
			Ref<DescriptorSet> m_Set = nullptr;
			glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };

		public:
			SandboxLayer() : Layer("SandboxLayer") {}
			~SandboxLayer() = default;

			void OnAttach() override 
			{
				KTN_INFO("Attaching...");

				m_Texture = TextureImporter::LoadTexture2D("Assets/Textures/checkerboard.png");

				m_Shader = Shader::Create("Assets/Shaders/ShaderTest.glsl");

				m_Set = DescriptorSet::Create({ 0, m_Shader });

				m_VAO = VertexArray::Create();


				float vertices[] = {
					// positions      // texcoords  // colors
					-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
					 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
					 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
					-0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f
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
				glm::vec3 dir{ 0.0f };
				float speed = 1.5f;

				if (Input::IsKeyPressed(Key::W))
					dir.y =  1;
				if (Input::IsKeyPressed(Key::A))
					dir.x = -1;
				if (Input::IsKeyPressed(Key::S))
					dir.y = -1;
				if (Input::IsKeyPressed(Key::D))
					dir.x =  1;

				m_Position += (glm::length(dir) > 0 ? glm::normalize(dir) : dir) * speed * (float)Time::GetDeltaTime();
			}
			void OnRender() override
			{
				auto commandBuffer = RendererCommand::GetCurrentCommandBuffer();

				m_Shader->Bind();

				m_Set->SetTexture("u_Texture", m_Texture);
				m_Set->Upload(commandBuffer);

				commandBuffer->BindSets(&m_Set);

				glm::mat4 model = glm::translate(glm::mat4(1.0f), m_Position) * glm::scale(glm::mat4(1.0f), { 0.5f, 0.5f, 0.5f});
				m_Shader->SetPushValue("Matrix", &model);
				m_Shader->BindPushConstants(commandBuffer);
				RendererCommand::DrawIndexed(m_VAO);
			}
			void OnImgui() override  { }
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