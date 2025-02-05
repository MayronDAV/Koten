#include "Koten/Koten.h"
#include "Koten/EntryPoint.h"

// lib
#include <imgui.h>


namespace KTN
{
	class SandboxLayer : public Layer
	{
		public:
			SandboxLayer() : Layer("SandboxLayer") {}
			~SandboxLayer() = default;

			void OnAttach() override 
			{
				KTN_INFO("Attaching...");

				// testing

				auto shader = Shader::Create("Assets/Shaders/ShaderTest.glsl");

				auto vao = VertexArray::Create();

				struct Vertex
				{
					glm::vec3 Position;
					glm::vec3 Color;
				};

				auto vbo = VertexBuffer::Create(4 * sizeof(Vertex));
				vbo->SetLayout({
					{ DataType::Float3 , "a_Position"	},
					{ DataType::Float3 , "a_Color"		},
				});
				vao->SetVertexBuffer(vbo);

				uint32_t indices[] = { 0, 1, 2, 3 };
				auto ebo = IndexBuffer::Create(indices, 4);
				vao->SetIndexBuffer(ebo);

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
			void OnRender() override {}
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