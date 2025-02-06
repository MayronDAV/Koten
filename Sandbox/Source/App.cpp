#include "Koten/Koten.h"
#include "Koten/EntryPoint.h"

// lib
#include <imgui.h>


namespace KTN
{
	class SandboxLayer : public Layer
	{
		private:
			Ref<Shader> m_Shader = nullptr;
			Ref<VertexArray> m_VAO = nullptr;

		public:
			SandboxLayer() : Layer("SandboxLayer") {}
			~SandboxLayer() = default;

			void OnAttach() override 
			{
				KTN_INFO("Attaching...");

				// testing

				m_Shader = Shader::Create("Assets/Shaders/ShaderTest.glsl");

				m_VAO = VertexArray::Create();

				float vertices[] = {
					// positions        // colors
					-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
					 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
					 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
					-0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f
				};

				auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
				vbo->SetLayout({
					{ DataType::Float3 , "a_Position"	},
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
				if (Input::IsKeyPressed(Key::W))
					KTN_CORE_INFO("W is pressed!");
				if (Input::IsKeyPressed(Key::A))
					KTN_CORE_INFO("A is pressed!");
				if (Input::IsKeyPressed(Key::S))
					KTN_CORE_INFO("S is pressed!");
				if (Input::IsKeyPressed(Key::D))
					KTN_CORE_INFO("D is pressed!");
			}
			void OnRender() override
			{
				m_Shader->Bind();

				RendererCommand::DrawIndexed(m_VAO);
			}
			void OnImgui() override  { ImGui::ShowDemoWindow(); }
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