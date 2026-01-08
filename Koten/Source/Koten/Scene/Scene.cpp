#include "ktnpch.h"
#include "Scene.h"
#include "Koten/Graphics/Renderer.h"
#include "Entity.h"
#include "Koten/Systems/B2Physics.h"
#include "Koten/Graphics/DebugRenderer.h"
#include "Koten/Script/ScriptEngine.h"
#include "Koten/Graphics/DFFont.h"
#include "Koten/Project/Project.h"



namespace KTN
{
	namespace
	{
		template <typename Component, typename Dependency>
		void AddDependency(entt::registry& p_Registry)
		{
			p_Registry.template on_construct<Component>().template connect<&entt::registry::get_or_emplace<Dependency>>();
		}

		template <typename T>
		static void CopyComponentIfExists(entt::entity p_Src, entt::entity p_Dest, entt::registry& p_SrcRegistry, entt::registry& p_DestRegistry)
		{
			KTN_PROFILE_FUNCTION_LOW();

			if (p_SrcRegistry.all_of<T>(p_Src))
			{
				auto srcComponent = p_SrcRegistry.get<T>(p_Src);
				p_DestRegistry.emplace_or_replace<T>(p_Dest, srcComponent);
			}
		}

		template <typename... Component>
		static void CopyEntity(entt::entity p_Src, entt::entity p_Dest, entt::registry& p_SrcRegistry, entt::registry& p_DestRegistry)
		{
			KTN_PROFILE_FUNCTION_LOW();

			(CopyComponentIfExists<Component>(p_Src, p_Dest, p_SrcRegistry, p_DestRegistry), ...);
		}
	}

	Scene::Scene()
	{
		KTN_PROFILE_FUNCTION();

		AddDependency<SpriteComponent, TransformComponent>(m_Registry);
		AddDependency<LineRendererComponent, TransformComponent>(m_Registry);
		AddDependency<TextRendererComponent, TransformComponent>(m_Registry);
		AddDependency<CameraComponent, TransformComponent>(m_Registry);
		AddDependency<Rigidbody2DComponent, TransformComponent>(m_Registry);
		AddDependency<Rigidbody2DComponent, BodyShape2DComponent>(m_Registry);
		AddDependency<CharacterBody2DComponent, TransformComponent>(m_Registry);
		AddDependency<CharacterBody2DComponent, BodyShape2DComponent>(m_Registry);
		AddDependency<StaticBody2DComponent, TransformComponent>(m_Registry);
		AddDependency<StaticBody2DComponent, BodyShape2DComponent>(m_Registry);

		m_SystemManager = CreateUnique<SystemManager>();
		if (m_Config.UsePhysics2D)
			m_SystemManager->RegisterSystem<B2Physics>();

		m_SceneGraph = CreateUnique<SceneGraph>();
		m_SceneGraph->Init(m_Registry);
	}

	Scene::~Scene()
	{
	}

	void Scene::Copy(const Ref<Scene>& p_Src, const Ref<Scene>& p_Dest)
	{
		KTN_PROFILE_FUNCTION();

		p_Dest->Handle = p_Src->Handle;
		p_Dest->m_Width = p_Src->m_Width;
		p_Dest->m_Height = p_Src->m_Height;
		p_Dest->m_RenderTarget = p_Src->m_RenderTarget;
		p_Dest->m_Projection = p_Src->m_Projection;
		p_Dest->m_View = p_Src->m_View;
		p_Dest->m_ClearColor = p_Src->m_ClearColor;
		p_Dest->m_HaveCamera = p_Src->m_HaveCamera;

		auto& srcRegistry = p_Src->m_Registry;
		auto& destRegistry = p_Dest->m_Registry;
		std::unordered_map<UUID, std::pair<entt::entity, entt::entity>> enttMap;

		p_Dest->m_SceneGraph->DisableOnConstruct(destRegistry, true);

		srcRegistry.view<IDComponent, TagComponent>().each(
		[&](auto p_Entity, IDComponent& p_ID, TagComponent& p_Tag)
		{
			UUID uuid = p_ID.ID;
			const auto& name = p_Tag.Tag;

			Entity newEntity = p_Dest->CreateEntity(uuid, name);
			enttMap[uuid] = std::make_pair(p_Entity, newEntity.GetHandle());

			CopyEntity<ALL_COMPONENTS>(p_Entity, (entt::entity)newEntity, srcRegistry, destRegistry);

			auto hcomp = newEntity.TryGetComponent<HierarchyComponent>();
			if (hcomp)
			{
				hcomp->Parent = entt::null;
				hcomp->First = entt::null;
				hcomp->Prev = entt::null;
				hcomp->Next = entt::null;
			}
		});

		destRegistry.view<IDComponent, HierarchyComponent>().each(
		[&](auto p_Entity, IDComponent& p_ID, HierarchyComponent& p_HC)
		{
			auto srcEntt = enttMap[p_ID.ID].first;

			auto& srcHC = srcRegistry.get<HierarchyComponent>(srcEntt);
			p_HC.Parent = srcHC.Parent != entt::null ? enttMap[srcRegistry.get<IDComponent>(srcHC.Parent).ID].second : entt::null;
			p_HC.First  = srcHC.First != entt::null ? enttMap[srcRegistry.get<IDComponent>(srcHC.First).ID].second : entt::null;
			p_HC.Prev   = srcHC.Prev != entt::null ? enttMap[srcRegistry.get<IDComponent>(srcHC.Prev).ID].second : entt::null;
			p_HC.Next   = srcHC.Next != entt::null ? enttMap[srcRegistry.get<IDComponent>(srcHC.Next).ID].second : entt::null;
			p_HC.ChildCount = srcHC.ChildCount;
		});

		p_Dest->m_SceneGraph->DisableOnConstruct(destRegistry, false);
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
		entt.AddComponent<RuntimeComponent>();
		m_EntityMap[p_UUID] = (entt::entity)entt;
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
			m_Registry.view<RuntimeComponent, TransformComponent>().each(
			[&](auto p_Entity, const RuntimeComponent& p_Runtime, const TransformComponent& p_Transform)
			{
				if (!p_Runtime.Active) return;

				auto& settings = Engine::Get().GetSettings();
				auto shape2d = m_Registry.try_get<BodyShape2DComponent>(p_Entity);
				if (shape2d && settings.ShowDebugPhysicsCollider)
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
					command.Render2D.Texture = As<Asset, Texture2D>(AssetManager::Get()->GetAsset(sprite->Texture));
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

				auto text = m_Registry.try_get<TextRendererComponent>(p_Entity);
				if (text)
				{
					TextParams params = {};
					params.Color = text->Color;
					params.BgColor = text->BgColor;
					params.CharBgColor = text->CharBgColor;
					params.DrawBg = text->DrawBg;
					params.LineSpacing = text->LineSpacing;
					params.Kerning = text->Kerning;

					Renderer::SubmitString(text->String, As<Asset, DFFont>(AssetManager::Get()->GetAsset(text->Font)), p_Transform.GetWorldMatrix(), params, (int)p_Entity);
				}
			});
		}
		Renderer::End();
	}

	void Scene::OnSimulationStart()
	{
		KTN_PROFILE_FUNCTION();

		m_SystemManager->OnStart(this);
	}

	void Scene::OnSimulationStop()
	{
		KTN_PROFILE_FUNCTION();

		m_SystemManager->OnStop(this);
	}

	void Scene::OnUpdateSimulation()
	{
		KTN_PROFILE_FUNCTION();

		RemoveSystems();

		if (!m_IsPaused || (m_StepFrames >= 0 && m_StepFrames-- > 0))
		{
			m_SystemManager->OnUpdate(this);

			m_SceneGraph->Update(m_Registry);
		}
	}

	void Scene::OnRuntimeStart()
	{
		KTN_PROFILE_FUNCTION();

		m_SystemManager->OnStart(this);

		ScriptEngine::OnRuntimeStart(this);
	}

	void Scene::OnRuntimeStop()
	{
		KTN_PROFILE_FUNCTION();

		ScriptEngine::OnRuntimeStop();

		m_SystemManager->OnStop(this);
	}

	void Scene::SetViewportSize(uint32_t p_Width, uint32_t p_Height)
	{
		KTN_PROFILE_FUNCTION();

		m_Width = p_Width;
		m_Height = p_Height;
	}

	void Scene::Step(int p_Frames)
	{
		m_StepFrames = p_Frames;
	}

	Entity Scene::GetEntityByUUID(UUID p_UUID)
	{
		KTN_PROFILE_FUNCTION();

		auto it = m_EntityMap.find(p_UUID);
		if (it != m_EntityMap.end())
			return { it->second, this };

		return Entity();
	}

	Entity Scene::GetEntityByTag(const std::string& p_Tag)
	{
		KTN_PROFILE_FUNCTION();

		Entity entt = {};
		m_Registry.view<TagComponent>().each(
		[&](auto p_Entity, const TagComponent& p_Tc)
		{
			if (p_Tc.Tag == p_Tag)
			{
				entt = Entity{ p_Entity, this };
				return;
			}
		});

		return entt;
	}

	void Scene::RemoveSystems()
	{
		KTN_PROFILE_FUNCTION();

		if (!m_Config.UsePhysics2D && m_SystemManager->HasSystem<B2Physics>())
		{
			m_SystemManager->GetSystem<B2Physics>()->OnStop(this);
			m_SystemManager->RemoveSystem<B2Physics>();
		}
	}

	void Scene::OnUpdateRuntime()
	{
		KTN_PROFILE_FUNCTION();

		RemoveSystems();

		if (!m_IsPaused || (m_StepFrames >= 0 && m_StepFrames-- > 0))
		{
			m_SystemManager->OnUpdate(this);
			
			ScriptEngine::OnRuntimeUpdate(this);

			m_SceneGraph->Update(m_Registry);
		}

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

				m_Projection = p_Camera.Camera.GetProjection();
				m_View = glm::inverse(p_Transform.GetWorldMatrix());
				m_ClearColor = p_Camera.ClearColor;
				m_HaveCamera = true;
				first = false;
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
			m_Registry.view<RuntimeComponent, TransformComponent>().each(
			[&](auto p_Entity, const RuntimeComponent& p_Runtime, const TransformComponent& p_Transform)
			{
				if (!p_Runtime.Active) return;

				auto& settings = Engine::Get().GetSettings();
				auto shape2d = m_Registry.try_get<BodyShape2DComponent>(p_Entity);
				if (shape2d && settings.ShowDebugPhysicsCollider)
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
					command.Render2D.Texture = As<Asset, Texture2D>(AssetManager::Get()->GetAsset(sprite->Texture));
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

				auto text = m_Registry.try_get<TextRendererComponent>(p_Entity);
				if (text)
				{
					TextParams params = {};
					params.Color = text->Color;
					params.BgColor = text->BgColor;
					params.CharBgColor = text->CharBgColor;
					params.DrawBg = text->DrawBg;
					params.LineSpacing = text->LineSpacing;
					params.Kerning = text->Kerning;

					Renderer::SubmitString(text->String, As<Asset, DFFont>(AssetManager::Get()->GetAsset(text->Font)), p_Transform.GetWorldMatrix(), params, (int)p_Entity);
				}
			});
		}
		Renderer::End();
	}

} // namespace KTN
