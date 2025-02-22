#include "ktnpch.h"
#include "Scene.h"
#include "Koten/Graphics/Renderer.h"
#include "Entity.h"



namespace KTN
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& p_Tag)
	{
		auto entt = Entity(m_Registry.create(), this);
		entt.AddComponent<TagComponent>(p_Tag);
		return entt;
	}

	void Scene::OnUpdate()
	{
	}

	void Scene::OnRender()
	{
		m_Registry.view<TransformComponent, SpriteComponent>().each(
		[&](auto p_Entity, const TransformComponent& p_Transform, const SpriteComponent& p_Sprite)
		{
			RenderCommand command = {};
			command.Transform = p_Transform.GetWorldMatrix();
			command.SpriteData.Color = p_Sprite.Color;
			command.SpriteData.Texture = p_Sprite.Texture;

			Renderer::Submit(command);
		});
	}

} // namespace KTN
