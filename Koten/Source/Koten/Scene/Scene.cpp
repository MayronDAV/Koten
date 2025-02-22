#include "ktnpch.h"
#include "Scene.h"
#include "Koten/Graphics/Renderer.h"



namespace KTN
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	entt::entity Scene::CreateEntity()
	{
		return m_Registry.create();
	}

	void Scene::OnUpdate()
	{
	}

	void Scene::OnRender()
	{
		m_Registry.view<TagComponent, TransformComponent, SpriteComponent>().each(
		[&](auto p_Entity, const TagComponent& p_Tag, const TransformComponent& p_Transform, const SpriteComponent& p_Sprite)
		{
			RenderCommand command = {};
			command.Transform = p_Transform.GetWorldMatrix();
			command.SpriteData.Color = p_Sprite.Color;
			command.SpriteData.Texture = p_Sprite.Texture;

			Renderer::Submit(command);
		});
	}

} // namespace KTN
