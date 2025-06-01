#pragma once
#include "Koten/Core/Base.h"
#include "Components.h"
#include "SceneGraph.h"
#include "SystemManager.h"

// lib
#include <entt/entt.hpp>

// std
#include <string>



namespace KTN
{
	class KTN_API Entity;

	class KTN_API Scene
	{
		public:
			Scene();
			Scene(const Scene&) = default;
			~Scene();

			static void Copy(const Ref<Scene>& p_Src, const Ref<Scene>& p_Dest);
			static Ref<Scene> Copy(const Ref<Scene>& p_Scene);

			Entity CreateEntity(const std::string& p_Tag = std::string());
			Entity CreateEntity(UUID p_UUID, const std::string& p_Tag = std::string());

			void OnUpdate();
			void OnRender(const glm::mat4& p_Projection, const glm::mat4& p_View, const glm::vec4& p_ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f });

			void OnSimulationStart();
			void OnSimulationStop();
			void OnUpdateSimulation();

			void OnRuntimeStart();
			void OnRuntimeStop();

			void OnUpdateRuntime();
			void OnRenderRuntime();

			void SetRenderTarget(const Ref<Texture2D>& p_Target) { m_RenderTarget = p_Target; }
			void SetViewportSize(uint32_t p_Width, uint32_t p_Height, bool p_Runtime = false);
			void SetIsPaused(bool p_Paused) { m_IsPaused = p_Paused; }

			void Step(int p_Frames = 1);

			bool IsPaused() const { return m_IsPaused; }
			Entity GetEntityByUUID(UUID p_UUID);
			Entity GetEntityByTag(const std::string& p_Tag);
			Unique<SystemManager>& GetSystemManager() { return m_SystemManager; }

			entt::registry& GetRegistry() { return m_Registry; }

		private:
			entt::registry m_Registry;
			Ref<Texture2D> m_RenderTarget = nullptr;
			uint32_t m_Width = 0, m_Height = 0;
			glm::mat4 m_Projection{ 1.0f };
			glm::mat4 m_View{ 1.0f };
			glm::vec4 m_ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
			bool m_HaveCamera = false;
			bool m_IsPaused = false;
			int m_StepFrames = 0;
			Unique<SystemManager> m_SystemManager = nullptr;

			std::unordered_map<UUID, entt::entity> m_EntityMap;

			Unique<SceneGraph> m_SceneGraph = nullptr;

			friend class Entity;
	};


} // namespace KTN