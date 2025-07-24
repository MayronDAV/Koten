#include "Koten/Koten.h"
#include "Koten/EntryPoint.h"

// lib
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



namespace KTN
{
	class RuntimeLayer : public Layer
	{
		private:
			Ref<AssetManager> m_AssetManager = nullptr;
			Ref<Project> m_Project = nullptr;

		public:
			RuntimeLayer() : Layer("RuntimeLayer")
			{
				m_Project = Project::GetActive();
				m_AssetManager = AssetManager::Create();
				SceneManager::Init();
			}

			~RuntimeLayer() = default;

			void OnAttach() override 
			{
				KTN_PROFILE_FUNCTION();

				ScriptEngine::CompileLoadAppAssembly();

				bool success = m_AssetManager->DeserializeAssetPack();
				KTN_VERIFY(success, "Failed to load asset pack!");

				if (m_Project->GetConfig().StartScene != 0)
					SceneManager::Load(m_Project->GetConfig().StartScene);

				SceneManager::SetRenderTarget(nullptr);
				SceneManager::Play();
			}

			void OnDetach() override
			{
				KTN_PROFILE_FUNCTION();

				SceneManager::Stop();
			}

			void OnUpdate() override
			{
				KTN_PROFILE_FUNCTION();

				auto& window = Application::Get().GetWindow();
				SceneManager::SetViewportSize(window->GetWidth(), window->GetHeight());
				SceneManager::OnUpdate();
			}

			void OnRender() override
			{
				KTN_PROFILE_FUNCTION();

				SceneManager::OnRender();
			}
	};



	Application* CreateApplication(int p_Argc, char** p_Argv)
	{
		KTN_PROFILE_FUNCTION();

		auto project = Project::LoadRuntime("Data.ktdt");
		KTN_VERIFY(project, "Failed to load Data.ktdt!");
		auto& config = project->GetConfig();

		ApplicationConfig appConfig = {};
		appConfig.Title = config.Name;
		appConfig.IconPath = config.IconPath;

		auto app = new Application(appConfig);
		app->PushLayer(CreateRef<RuntimeLayer>());
		return app;
	}

} // namespace KTN