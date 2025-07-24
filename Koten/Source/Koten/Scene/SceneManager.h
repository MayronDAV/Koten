#pragma once
#include "Scene.h"


namespace KTN
{
	enum class LoadMode
	{
		Single = 0,
		Additive
	};

	struct SceneManagerConfig
	{
		LoadMode Mode = LoadMode::Single;
		bool CopyScenesOnPlay = false;
	};

	class KTN_API SceneManager
	{
		public:
			static void Init(const SceneManagerConfig& p_Config = {});
			static void Shutdown();

			static void SetRenderTarget(const Ref<Texture2D>& p_Target);
			static void SetViewportSize(uint32_t p_Width, uint32_t p_Height);

			static void Play();
			static void Simulate();
			static void Pause(bool p_Value = true);
			static void Stop();
			static void Step(int p_Frames = 1);

			static void OnUpdate();
			static void OnRender(const glm::mat4& p_Projection = glm::mat4(1.0f), const glm::mat4& p_View = glm::mat4(1.0f), const glm::vec4& p_ClearColor = {0.0f, 0.0f, 0.0f, 1.0f});

			static bool Load(AssetHandle p_Handle) { return Load(p_Handle, GetConfig().Mode); }
			static bool Load(AssetHandle p_Handle, LoadMode p_Mode);
			static void Unload(AssetHandle p_Handle);

			static AssetHandle New();
			static bool Save(AssetHandle p_Handle);
			static void SaveAs(AssetHandle p_Handle, const std::string& p_Path);

			static const std::vector<Ref<Scene>>& GetLoadedScenes();
			static const std::vector<Ref<Scene>>& GetActiveScenes();
			static bool IsPaused();
			static Entity GetEntityByUUID(UUID p_UUID);
			static Entity GetEntityByTag(const std::string& p_Tag);
			static SceneManagerConfig& GetConfig();
	};


} // namespace KTN