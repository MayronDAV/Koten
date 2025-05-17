#include "ktnpch.h"
#include "Scene.h"
#include "Koten/Graphics/Renderer.h"
#include "Entity.h"
#include "SystemManager.h"
#include "Koten/Physics/Box2D/B2Physics.h"
#include "Koten/Graphics/DebugRenderer.h"



namespace KTN
{
	namespace
	{
		template <typename Component, typename Dependency>
		void AddDependency(entt::registry& p_Registry)
		{
			p_Registry.template on_construct<Component>().template connect<&entt::registry::get_or_emplace<Dependency>>();
		}

		template<typename... Component>
		struct ComponentGroup
		{
		};

		using AllComponents =
			ComponentGroup<HierarchyComponent, TransformComponent, SpriteComponent, LineRendererComponent,
			CameraComponent, Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent>;

		template<typename... Component>
		static void CopyComponent(entt::registry& p_Dest, entt::registry& p_Src, const std::unordered_map<UUID, entt::entity>& p_Map)
		{
			([&]()
			{
				auto view = p_Src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity destEntity = p_Map.at(p_Src.get<IDComponent>(srcEntity).ID);

					auto& srcComponent = p_Src.get<Component>(srcEntity);
					p_Dest.emplace_or_replace<Component>(destEntity, srcComponent);
				}
			}(), ...);
		}

		template<typename... Component>
		static void CopyComponent(ComponentGroup<Component...>, entt::registry& p_Dest, entt::registry& p_Src, const std::unordered_map<UUID, entt::entity>& p_Map)
		{
			CopyComponent<Component...>(p_Dest, p_Src, p_Map);
		}
	}

	Scene::Scene()
	{
		KTN_PROFILE_FUNCTION();

		AddDependency<SpriteComponent, TransformComponent>(m_Registry);
		AddDependency<LineRendererComponent, TransformComponent>(m_Registry);
		AddDependency<CameraComponent, TransformComponent>(m_Registry);
		AddDependency<Rigidbody2DComponent, TransformComponent>(m_Registry);
		AddDependency<BoxCollider2DComponent, Rigidbody2DComponent>(m_Registry);
		AddDependency<CircleCollider2DComponent, Rigidbody2DComponent>(m_Registry);

		m_SceneGraph = CreateUnique<SceneGraph>();
		m_SceneGraph->Init(m_Registry);
	}

	Scene::~Scene()
	{
	}

	void Scene::Copy(const Ref<Scene>& p_Src, const Ref<Scene>& p_Dest)
	{
		KTN_PROFILE_FUNCTION();

		p_Dest->m_Width = p_Src->m_Width;
		p_Dest->m_Height = p_Src->m_Height;
		p_Dest->m_RenderTarget = p_Src->m_RenderTarget;
		p_Dest->m_Projection = p_Src->m_Projection;
		p_Dest->m_View = p_Src->m_View;
		p_Dest->m_ClearColor = p_Src->m_ClearColor;
		p_Dest->m_HaveCamera = p_Src->m_HaveCamera;

		auto& srcRegistry = p_Src->m_Registry;
		auto& destRegistry = p_Dest->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		srcRegistry.view<IDComponent, TagComponent>().each(
		[&](auto p_Entity, IDComponent& p_ID, TagComponent& p_Tag)
		{
			UUID uuid = p_ID.ID;
			const auto& name = p_Tag.Tag;

			Entity newEntity = p_Dest->CreateEntity(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		});

		CopyComponent(AllComponents{}, destRegistry, srcRegistry, enttMap);
	}

	Ref<Scene> Scene::Copy(const Ref<Scene>& p_Scene)
	{
		KTN_PROFILE_FUNCTION();

		Ref<Scene> newScene = CreateRef<Scene>();
		Copy(p_Scene, newScene);
		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& p_Tag)
	{
		KTN_PROFILE_FUNCTION();

		return CreateEntity(UUID(), p_Tag);
	}

	Entity Scene::CreateEntity(UUID p_UUID, const std::string& p_Tag)
	{
		KTN_PROFILE_FUNCTION();

		auto entt = Entity(m_Registry.create(), this);
		entt.AddComponent<IDComponent>(p_UUID);
		entt.AddComponent<TagComponent>(p_Tag.empty() ? "Entity" : p_Tag);
		return entt;
	}

	void Scene::OnUpdate()
	{
		KTN_PROFILE_FUNCTION();

		m_SceneGraph->Update(m_Registry);
	}

	void Scene::OnRender(const glm::mat4& p_Projection, const glm::mat4& p_View, const glm::vec4& p_ClearColor)
	{
		KTN_PROFILE_FUNCTION();

		RenderBeginInfo info = {};
		info.RenderTarget = m_RenderTarget;
		info.Width = m_Width;
		info.Height = m_Height;
		info.Projection = p_Projection;
		info.View = p_View;
		info.Clear = false;
		info.ClearColor = p_ClearColor;

		Renderer::Begin(info);
		{
			m_Registry.view<TransformComponent>().each(
			[&](auto p_Entity, const TransformComponent& p_Transform)
			{
				if (Engine::GetSettings().ShowDebugPhysicsCollider)
					DebugRenderer::DrawCollider2D({ p_Entity, this }, { 1.0f, 0.65f, 0.0f, 1.0f });

				RenderCommand command = {};
				command.EntityID = (int)p_Entity;
				command.Transform = p_Transform.GetWorldMatrix();

				auto sprite = m_Registry.try_get<SpriteComponent>(p_Entity);
				if (sprite)
				{
					command.Type = RenderType::R2D;
					command.Render2D.Type = sprite->Type;
					command.Render2D.Thickness = sprite->Thickness;
					command.Render2D.Fade = sprite->Fade;
					command.Render2D.Color = sprite->Color;
					command.Render2D.Texture = sprite->Texture;
					command.Render2D.Size = sprite->Size;
					command.Render2D.BySize = sprite->BySize;
					command.Render2D.Offset = sprite->Offset;
					command.Render2D.Scale = sprite->Scale;

					Renderer::Submit(command);
				}

				auto line = m_Registry.try_get<LineRendererComponent>(p_Entity);
				if (line)
				{
					command.Type = RenderType::Line;
					command.Line.Primitive = line->Primitive;
					command.Line.Color = line->Color;
					command.Line.Width = line->Width;
					command.Line.Start = line->Start;
					command.Line.End = line->End;

					Renderer::Submit(command);
				}
			});
		}
		Renderer::End();
	}

	void Scene::OnRuntimeStart()
	{
		KTN_PROFILE_FUNCTION();

		if (SystemManager::HasSystem<B2Physics>())
			SystemManager::GetSystem<B2Physics>()->OnStart(this);
	}

	void Scene::OnRuntimeStop()
	{
		KTN_PROFILE_FUNCTION();

		if (SystemManager::HasSystem<B2Physics>())
			SystemManager::GetSystem<B2Physics>()->OnStop(this);
	}

	void Scene::SetViewportSize(uint32_t p_Width, uint32_t p_Height, bool p_Runtime)
	{
		KTN_PROFILE_FUNCTION();

		m_Width = p_Width;
		m_Height = p_Height;
	}

	void Scene::OnUpdateRuntime()
	{
		KTN_PROFILE_FUNCTION();

		if (SystemManager::HasSystem<B2Physics>())
			SystemManager::GetSystem<B2Physics>()->SyncTransforms(this);

		m_SceneGraph->Update(m_Registry);

		bool first = true;
		m_Registry.view<TransformComponent, CameraComponent>().each(
		[&](auto p_Entt, TransformComponent& p_Transform, CameraComponent& p_Camera)
		{
			p_Camera.Camera.SetViewportSize(m_Width, m_Height);
			p_Camera.Camera.OnUpdate();

			if (p_Camera.Primary)
			{
				if (!first)
				{
					KTN_CORE_ERROR("there can only be one primary camera!");
					return;
				}

				m_Projection				= p_Camera.Camera.GetProjection();
				m_View						= glm::inverse(p_Transform.GetWorldMatrix());
				m_ClearColor				= p_Camera.ClearColor;
				m_HaveCamera				= true;
				first						= false;
			}
		});
	}

	void Scene::OnRenderRuntime()
	{
		KTN_PROFILE_FUNCTION();

		RenderBeginInfo info				= {};
		info.RenderTarget					= m_RenderTarget;
		info.Width							= m_Width;
		info.Height							= m_Height;
		info.Projection						= m_Projection;
		info.View							= m_View;
		info.Clear							= false;
		if (m_HaveCamera)
			info.ClearColor					= m_ClearColor;
		
		Renderer::Begin(info);
		{
			m_Registry.view<TransformComponent>().each(
			[&](auto p_Entity, const TransformComponent& p_Transform)
			{
				if (Engine::GetSettings().ShowDebugPhysicsCollider)
					DebugRenderer::DrawCollider2D({ p_Entity, this }, { 1.0f, 0.65f, 0.0f, 1.0f });

				RenderCommand command = {};
				command.EntityID = (int)p_Entity;
				command.Transform = p_Transform.GetWorldMatrix();

				auto sprite = m_Registry.try_get<SpriteComponent>(p_Entity);
				if (sprite)
				{
					command.Type = RenderType::R2D;
					command.Render2D.Type = sprite->Type;
					command.Render2D.Thickness = sprite->Thickness;
					command.Render2D.Fade = sprite->Fade;
					command.Render2D.Color = sprite->Color;
					command.Render2D.Texture = sprite->Texture;
					command.Render2D.Size = sprite->Size;
					command.Render2D.BySize = sprite->BySize;
					command.Render2D.Offset = sprite->Offset;
					command.Render2D.Scale = sprite->Scale;

					Renderer::Submit(command);
				}

				auto line = m_Registry.try_get<LineRendererComponent>(p_Entity);
				if (line)
				{
					command.Type = RenderType::Line;
					command.Line.Primitive = line->Primitive;
					command.Line.Color = line->Color;
					command.Line.Width = line->Width;
					command.Line.Start = line->Start;
					command.Line.End = line->End;

					Renderer::Submit(command);
				}
			});
		}
		Renderer::End();
	}

} // namespace KTN
