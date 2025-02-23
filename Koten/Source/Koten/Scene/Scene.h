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

			Entity CreateEntity(const std::string& p_Tag);

			void OnUpdate();
			void OnRender();

			void SetRenderTarget(const Ref<Texture2D>& p_Target) { m_RenderTarget = p_Target; }
			void SetViewportSize(uint32_t p_Width, uint32_t p_Height);

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