#pragma once
#include "Koten/Core/Base.h"
#include "Components.h"
#include "SceneGraph.h"

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

			void OnRuntimeStart();
			void OnRuntimeStop();

			void OnUpdateRuntime();
			void OnRenderRuntime();

			void SetRenderTarget(const Ref<Texture2D>& p_Target) { m_RenderTarget = p_Target; }
			void SetViewportSize(uint32_t p_Width, uint32_t p_Height, bool p_Runtime = false);

			entt::registry& GetRegistry() { return m_Registry; }

		private:
			entt::registry m_Registry;
			Ref<Texture2D> m_RenderTarget = nullptr;
			uint32_t m_Width = 0, m_Height = 0;
			glm::mat4 m_Projection{ 1.0f };
			glm::mat4 m_View{ 1.0f };
			glm::vec4 m_ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
			bool m_HaveCamera = false;

			Unique<SceneGraph> m_SceneGraph = nullptr;

			friend class Entity;
	};


} // namespace KTN