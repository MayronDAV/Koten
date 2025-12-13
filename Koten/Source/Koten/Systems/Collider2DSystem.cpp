#include "ktnpch.h"
#include "Collider2DSystem.h"
#include "Koten/Collision/AABBTree.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/Entity.h"
#include "Koten/Script/ScriptEngine.h"
#include "Koten/Asset/AssetManager.h"

// lib
#include <box2d/box2d.h>



namespace KTN
{
	static AABB ComputeAABB(const TransformComponent& p_Transform, const Collider2DComponent& p_Collider)
	{
		KTN_PROFILE_FUNCTION_LOW();

		auto pos = glm::vec2(p_Transform.GetWorldTranslation());
		auto scale = glm::vec2(p_Transform.GetWorldScale());
		glm::vec2 center = pos + p_Collider.Offset * scale;

		if (p_Collider.Shape == Collider2DShape::Box)
		{
			glm::vec2 size = p_Collider.Size * scale;

			return {
				center - size ,
				center + size
			};
		}
		else if (p_Collider.Shape == Collider2DShape::Circle)
		{
			float radius = p_Collider.Size.x * scale.x;

			return {
				center - glm::vec2(radius),
				center + glm::vec2(radius)
			};
		}
	}


	Collider2DSystem::Collider2DSystem()
	{
		m_BroadPhase = CreateRef<AABBTree>();
	}

	Collider2DSystem::~Collider2DSystem()
	{
	}

	bool Collider2DSystem::OnStart(Scene* p_Scene)
	{
		KTN_PROFILE_FUNCTION();

		auto& registry = p_Scene->GetRegistry();
		registry.view<TransformComponent, Collider2DComponent>().each(
		[&](auto p_Entt, TransformComponent& p_Transform, Collider2DComponent& p_Col)
		{
			Entity entt{ p_Entt, p_Scene };
			AABB aabb = ComputeAABB(p_Transform, p_Col);
			p_Col.NodeIndex = m_BroadPhase->CreateNode(entt.GetUUID(), aabb);
		});

		return true;
	}

	bool Collider2DSystem::OnStop(Scene* p_Scene)
	{
		m_BroadPhase->Reset();

		return true;
	}

	bool Collider2DSystem::OnInit()
	{
		return true;
	}

	void Collider2DSystem::OnUpdate(Scene* p_Scene)
	{

	}

	void Collider2DSystem::SyncBroadPhase(Scene* p_Scene, float p_Dt)
	{
		KTN_PROFILE_FUNCTION();

		if (m_Paused) return;

		auto& registry = p_Scene->GetRegistry();
		registry.view<TransformComponent, Collider2DComponent>().each(
		[&](auto p_Entt, TransformComponent& p_Transform, Collider2DComponent& p_Col)
		{
			Entity entt = { p_Entt, p_Scene };

			if (entt.HasComponent<Rigidbody2DComponent>())
			{
				auto& r2d = entt.GetComponent<Rigidbody2DComponent>();

				if (r2d.Body == B2BodyID{}) return;
				if (r2d.Type == Rigidbody2DComponent::BodyType::Static) return;

				b2BodyId body = {
					.index1 = r2d.Body.Index,
					.world0 = r2d.Body.World,
					.generation = r2d.Body.Generation
				};

				AABB newAabb = ComputeAABB(p_Transform, p_Col);
				b2Vec2 vel = b2Body_GetLinearVelocity(body);
				float dt = (float)Time::GetDeltaTime();
				glm::vec2 displacement = {
					vel.x * dt,
					vel.y * dt
				};

				m_BroadPhase->MoveNode(p_Col.NodeIndex, newAabb, displacement, false);
				return;
			}

			AABB oldAabb = m_BroadPhase->GetAABB(p_Col.NodeIndex);
			AABB newAabb = ComputeAABB(p_Transform, p_Col);
			glm::vec2 displacement = newAabb.Center() - oldAabb.Center();
			m_BroadPhase->MoveNode(p_Col.NodeIndex, newAabb, displacement, false);
		});
	}


} // namespace KTN