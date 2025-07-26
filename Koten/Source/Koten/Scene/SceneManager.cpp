#include "ktnpch.h"
#include "SceneManager.h"
#include "Entity.h"
#include "Koten/Asset/AssetManager.h"
#include "Koten/Project/Project.h"
#include "Koten/Asset/SceneImporter.h"
#include "Koten/Graphics/Renderer.h"



namespace KTN
{
	enum class RuntimeState
	{
		None,
		Play,
		Simulate
	};

	namespace
	{
		struct Data
		{
			SceneManagerConfig Config = {};

			RuntimeState State = RuntimeState::None;
			bool IsPaused = false;

			std::vector<Ref<Scene>> ScenesToStop;

			std::vector<Ref<Scene>> Scenes;
			std::vector<Ref<Scene>> ScenesCopy;
		};

		static Data* s_Data = nullptr;
	} // namespace

	void SceneManager::Init(const SceneManagerConfig& p_Config)
	{
		KTN_PROFILE_FUNCTION();

		s_Data = new Data();
		s_Data->Config = p_Config;
	}

	void SceneManager::Shutdown()
	{
		KTN_PROFILE_FUNCTION();

		if (s_Data)
		{
			delete s_Data;
			s_Data = nullptr;
		}
	}

	void SceneManager::SetRenderTarget(const Ref<Texture2D>& p_Target)
	{
		KTN_PROFILE_FUNCTION();

		for (size_t i = 0; i < s_Data->Scenes.size(); i++)
		{
			Ref<Scene> scene = s_Data->Config.CopyScenesOnPlay && s_Data->State != RuntimeState::None ? s_Data->ScenesCopy.at(i) : s_Data->Scenes.at(i);
			scene->SetRenderTarget(p_Target);
		}
	}

	void SceneManager::SetViewportSize(uint32_t p_Width, uint32_t p_Height)
	{
		KTN_PROFILE_FUNCTION();

		for (size_t i = 0; i < s_Data->Scenes.size(); i++)
		{
			Ref<Scene> scene = s_Data->Config.CopyScenesOnPlay && s_Data->State != RuntimeState::None ? s_Data->ScenesCopy.at(i) : s_Data->Scenes.at(i);
			scene->SetViewportSize(p_Width, p_Height);
		}
	}

	void SceneManager::Play()
	{
		KTN_PROFILE_FUNCTION();

		Stop();

		s_Data->State = RuntimeState::Play;

		for (const auto& scene : s_Data->Scenes)
		{
			if (s_Data->Config.CopyScenesOnPlay)
			{
				s_Data->ScenesCopy.push_back(Scene::Copy(scene));
				s_Data->ScenesCopy.back()->OnRuntimeStart();
				continue;
			}

			scene->OnRuntimeStart();
		}
	}

	void SceneManager::Simulate()
	{
		KTN_PROFILE_FUNCTION();

		Stop();

		s_Data->State = RuntimeState::Simulate;

		for (const auto& scene : s_Data->Scenes)
		{
			if (s_Data->Config.CopyScenesOnPlay)
			{
				s_Data->ScenesCopy.push_back(Scene::Copy(scene));
				s_Data->ScenesCopy.back()->OnSimulationStart();
				continue;
			}

			scene->OnSimulationStart();
		}
	}

	void SceneManager::Pause(bool p_Value)
	{
		KTN_PROFILE_FUNCTION();

		if (s_Data->State == RuntimeState::None)
			return;

		for (size_t i = 0; i < s_Data->Scenes.size(); i++)
		{
			Ref<Scene> scene = s_Data->Config.CopyScenesOnPlay ? s_Data->ScenesCopy.at(i) : s_Data->Scenes.at(i);
			scene->SetIsPaused(p_Value);
		}
		s_Data->IsPaused = true;
	}

	void SceneManager::Stop()
	{
		KTN_PROFILE_FUNCTION();

		if (s_Data->State == RuntimeState::None)
			return;

		for (size_t i = 0; i < s_Data->Scenes.size(); i++)
		{
			Ref<Scene> scene = s_Data->Config.CopyScenesOnPlay ? s_Data->ScenesCopy.at(i) : s_Data->Scenes.at(i);
			if (s_Data->State == RuntimeState::Play)
				scene->OnRuntimeStop();
			if (s_Data->State == RuntimeState::Simulate)
				scene->OnSimulationStop();
		}
		s_Data->ScenesCopy.clear();
		s_Data->State = RuntimeState::None;
	}

	void SceneManager::Step(int p_Frames)
	{
		KTN_PROFILE_FUNCTION();

		if (!s_Data->IsPaused)
			return;

		for (size_t i = 0; i < s_Data->Scenes.size(); i++)
		{
			Ref<Scene> scene = s_Data->Config.CopyScenesOnPlay && s_Data->State != RuntimeState::None ? s_Data->ScenesCopy.at(i) : s_Data->Scenes.at(i);
			scene->Step(p_Frames);
		}
	}

	void SceneManager::OnUpdate()
	{
		KTN_PROFILE_FUNCTION();

		for (auto& scene : s_Data->ScenesToStop)
		{
			switch (s_Data->State)
			{
				case RuntimeState::Play:
					scene->OnRuntimeStop();
					break;

				case RuntimeState::Simulate:
					scene->OnSimulationStop();
					break;
			}
			scene = nullptr;
		}
		s_Data->ScenesToStop.clear();

		for (size_t i = 0; i < s_Data->Scenes.size(); i++)
		{
			Ref<Scene> scene = s_Data->Config.CopyScenesOnPlay && s_Data->State != RuntimeState::None ? s_Data->ScenesCopy.at(i) : s_Data->Scenes.at(i);

			switch (s_Data->State)
			{
				case RuntimeState::Play:
					scene->OnUpdateRuntime();
					break;
				case RuntimeState::Simulate:
					scene->OnUpdateSimulation();
					break;
				default:
					scene->OnUpdate();
					break;
			}
		}
	}

	void SceneManager::OnRender(const glm::mat4& p_Projection, const glm::mat4& p_View, const glm::vec4& p_ClearColor)
	{
		KTN_PROFILE_FUNCTION();

		Renderer::Clear();

		for (size_t i = 0; i < s_Data->Scenes.size(); i++)
		{
			Ref<Scene> scene = s_Data->Config.CopyScenesOnPlay && s_Data->State != RuntimeState::None ? s_Data->ScenesCopy.at(i) : s_Data->Scenes.at(i);

			switch (s_Data->State)
			{
				case RuntimeState::Play:
					scene->OnRenderRuntime();
					break;
				default:
					scene->OnRender(p_Projection, p_View, p_ClearColor);
					break;
			}
		}
	}

	bool SceneManager::Load(AssetHandle p_Handle, LoadMode p_Mode)
	{
		KTN_PROFILE_FUNCTION();

		if (!AssetManager::Get()->IsAssetHandleValid(p_Handle))
		{
			KTN_CORE_ERROR("Handle {} isn't valid!", (uint64_t)p_Handle);
			return false;
		}

		s_Data->Config.Mode = p_Mode;

		auto state = s_Data->State;
		if (p_Mode == LoadMode::Single && state != RuntimeState::None && !s_Data->Scenes.empty())
		{
			s_Data->ScenesToStop.resize(s_Data->Scenes.size());
			for (size_t i = 0; i < s_Data->Scenes.size(); i++)
			{
				Ref<Scene> scene = s_Data->Config.CopyScenesOnPlay ? s_Data->ScenesCopy.at(i) : s_Data->Scenes.at(i);
				s_Data->ScenesToStop.push_back(scene);
			}
		}

		if (p_Mode == LoadMode::Single)
		{
			s_Data->Scenes.clear();
			s_Data->ScenesCopy.clear();
		}

		s_Data->Scenes.push_back(AssetManager::Get()->GetAsset<Scene>(p_Handle));

		if (state == RuntimeState::None)
			return true;

		auto& scene = s_Data->Scenes.back();
		if (state == RuntimeState::Play)
		{
			if (s_Data->Config.CopyScenesOnPlay)
			{
				s_Data->ScenesCopy.push_back(Scene::Copy(scene));
				s_Data->ScenesCopy.back()->OnRuntimeStart();
				return true;
			}

			scene->OnRuntimeStart();
		}

		if (state == RuntimeState::Play)
		{
			if (s_Data->Config.CopyScenesOnPlay)
			{
				s_Data->ScenesCopy.push_back(Scene::Copy(scene));
				s_Data->ScenesCopy.back()->OnSimulationStart();
				return true;
			}

			scene->OnSimulationStart();
		}

		return true;
	}

	void SceneManager::Unload(AssetHandle p_Handle)
	{
		KTN_PROFILE_FUNCTION();

		{
			auto it = std::find_if(s_Data->Scenes.begin(), s_Data->Scenes.end(), [&](const Ref<Scene>& p_Scene)
			{
				return p_Scene->Handle == p_Handle;
			});

			if (it == s_Data->Scenes.end())
			{
				KTN_CORE_ERROR("This scene handle isn't loaded!");
				return;
			}

			auto& scene = *it;
			if (s_Data->State != RuntimeState::None)
				s_Data->ScenesToStop.push_back(scene);
			s_Data->Scenes.erase(it);
		}

		if (s_Data->Config.CopyScenesOnPlay && !s_Data->ScenesCopy.empty())
		{
			auto it = std::find_if(s_Data->ScenesCopy.begin(), s_Data->ScenesCopy.end(), [&](const Ref<Scene>& p_Scene)
			{
				return p_Scene->Handle == p_Handle;
			});

			if (it == s_Data->ScenesCopy.end())
			{
				KTN_CORE_ERROR("This scene handle isn't loaded!");
				return;
			}

			auto& scene = *it;
			if (s_Data->State != RuntimeState::None)
				s_Data->ScenesToStop.push_back(scene);
			s_Data->ScenesCopy.erase(it);
		}
	}

	AssetHandle SceneManager::New()
	{
		KTN_PROFILE_FUNCTION();

		auto scene = CreateRef<Scene>();
		s_Data->Scenes.push_back(scene);
		AssetMetadata metadata = {};
		metadata.Type = AssetType::Scene;
		metadata.FilePath = "";
		AssetHandle handle;
		if (AssetManager::Get()->ImportAsset(handle, metadata, scene))
			return handle;

		KTN_CORE_ERROR("Failed to create new scene!");
		return 0;
	}

	bool SceneManager::Save(AssetHandle p_Handle)
	{
		KTN_PROFILE_FUNCTION();

		std::string path = AssetManager::Get()->GetMetadata(p_Handle).FilePath;
		if (path.empty())
			path = Project::GetAssetFileSystemPath("Scenes/Untitled.ktscn").string();
		SaveAs(p_Handle, path);
		return true;
	}

	void SceneManager::SaveAs(AssetHandle p_Handle, const std::string& p_Path)
	{
		KTN_PROFILE_FUNCTION();

		auto scene = AssetManager::Get()->GetAsset<Scene>(p_Handle);
		if (!scene)
		{
			KTN_CORE_ERROR("Failed to save scene, scene is null");
			return;
		}
		SceneImporter::SaveScene(scene, p_Path);
	}

	const std::vector<Ref<Scene>>& SceneManager::GetLoadedScenes()
	{
		KTN_PROFILE_FUNCTION();

		return s_Data->Scenes;
	}

	const std::vector<Ref<Scene>>& SceneManager::GetActiveScenes()
	{
		KTN_PROFILE_FUNCTION();

		if (s_Data->Config.CopyScenesOnPlay && s_Data->State != RuntimeState::None)
			return s_Data->ScenesCopy;

		return s_Data->Scenes;
	}

	bool SceneManager::IsPaused()
	{
		KTN_PROFILE_FUNCTION();

		return s_Data->IsPaused;
	}

	Entity SceneManager::GetEntityByUUID(UUID p_UUID)
	{
		KTN_PROFILE_FUNCTION();

		for (size_t i = 0; i < s_Data->Scenes.size(); i++)
		{
			Ref<Scene> scene = s_Data->Config.CopyScenesOnPlay && s_Data->State != RuntimeState::None ? s_Data->ScenesCopy.at(i) : s_Data->Scenes.at(i);

			auto entt = scene->GetEntityByUUID(p_UUID);
			if (entt)
				return entt;
		}

		return Entity();
	}

	Entity SceneManager::GetEntityByTag(const std::string& p_Tag)
	{
		KTN_PROFILE_FUNCTION();

		for (size_t i = 0; i < s_Data->Scenes.size(); i++)
		{
			Ref<Scene> scene = s_Data->Config.CopyScenesOnPlay && s_Data->State != RuntimeState::None ? s_Data->ScenesCopy.at(i) : s_Data->Scenes.at(i);

			auto entt = scene->GetEntityByTag(p_Tag);
			if (entt)
				return entt;
		}

		KTN_CORE_ERROR("This Tag doesn't exist!");
		return Entity();
	}

	SceneManagerConfig& SceneManager::GetConfig()
	{
		KTN_PROFILE_FUNCTION();

		return s_Data->Config;
	}


} // namespace KTN