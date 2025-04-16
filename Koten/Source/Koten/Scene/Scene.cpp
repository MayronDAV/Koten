#include "ktnpch.h"
#include "Scene.h"
#include "Koten/Graphics/Renderer.h"
#include "Entity.h"
#include "SystemManager.h"
#include "Koten/Physics/Box2D/B2Physics.h"



namespace KTN
{
	Scene::Scene()
	{
		KTN_PROFILE_FUNCTION();

		m_SceneGraph = CreateUnique<SceneGraph>();
		m_SceneGraph->Init(m_Registry);
	}

	Scene::~Scene()
	{
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
			m_Registry.view<TransformComponent, SpriteComponent>().each(
			[&](auto p_Entity, const TransformComponent& p_Transform, const SpriteComponent& p_Sprite)
			{
				RenderCommand command = {};
				command.EntityID = (int)p_Entity;
				command.Transform = p_Transform.GetWorldMatrix();
				command.SpriteData.Color = p_Sprite.Color;
				command.SpriteData.Texture = p_Sprite.Texture;

				Renderer::Submit(command);
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
			m_Registry.view<TransformComponent, SpriteComponent>().each(
			[&](auto p_Entity, const TransformComponent& p_Transform, const SpriteComponent& p_Sprite)
			{
				RenderCommand command		= {};
				command.EntityID			= (int)p_Entity;
				command.Transform			= p_Transform.GetWorldMatrix();
				command.SpriteData.Color	= p_Sprite.Color;
				command.SpriteData.Texture  = p_Sprite.Texture;

				Renderer::Submit(command);
			});
		}
		Renderer::End();
	}

} // namespace KTN
