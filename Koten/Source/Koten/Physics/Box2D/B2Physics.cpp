#include "ktnpch.h"
#include "B2Physics.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/SystemManager.h"
#include "Koten/Scene/Entity.h"

// lib
#include <box2d/box2d.h>



namespace KTN
{
	B2Physics::B2Physics()
	{
		KTN_PROFILE_FUNCTION();

	}

	B2Physics::~B2Physics()
	{
		KTN_PROFILE_FUNCTION();

		b2WorldId world = { .index1 = m_World.Index, .generation = m_World.Generation };
		b2DestroyWorld(world);
	}

	bool B2Physics::DestroyBody(const B2BodyID& p_Body)
	{
		KTN_PROFILE_FUNCTION();

		b2BodyId body = {
			.index1 = p_Body.Index,
			.world0 = p_Body.World,
			.generation = p_Body.Generation
		};

		b2DestroyBody(body);

		return true;
	}

	bool B2Physics::OnStart(Scene* p_Scene)
	{
		KTN_PROFILE_FUNCTION();

		auto& registry = p_Scene->GetRegistry();
		registry.view<TransformComponent, Rigidbody2DComponent>().each(
		[&](auto p_Entt, TransformComponent& p_Transform, Rigidbody2DComponent& p_RC)
		{
			Entity entt{ p_Entt, p_Scene };

			b2BodyDef bodyDef = b2DefaultBodyDef();
			switch (p_RC.Type)
			{
				case Rigidbody2DComponent::BodyType::Static:
					bodyDef.type = b2_staticBody;
					break;
				case Rigidbody2DComponent::BodyType::Dynamic:
					bodyDef.type = b2_dynamicBody;
					break;
				case Rigidbody2DComponent::BodyType::Kinematic:
					bodyDef.type = b2_kinematicBody;
					break;
			}

			glm::vec3 pos = p_Transform.GetWorldTranslation();
			glm::vec3 scale = p_Transform.GetWorldScale();
			glm::vec3 rot = p_Transform.GetWorldRotation();

			bodyDef.linearDamping = 1.0f;
			bodyDef.position = { pos.x, pos.y };
			bodyDef.fixedRotation = p_RC.FixedRotation;
			bodyDef.rotation = b2MakeRot(rot.z);

			b2WorldId world = { .index1 = m_World.Index, .generation = m_World.Generation };
			b2BodyId body = b2CreateBody(world, &bodyDef);
			p_RC.Body.Index = body.index1;
			p_RC.Body.World = body.world0;
			p_RC.Body.Generation = body.generation;

			if (entt.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entt.GetComponent<BoxCollider2DComponent>();

				b2Polygon box = b2MakeOffsetBox(bc2d.Size.x * scale.x, bc2d.Size.y * scale.y, b2Vec2(bc2d.Offset.x, bc2d.Offset.y),  b2MakeRot(0.0f));

				b2ShapeDef shapeDef = b2DefaultShapeDef();
				shapeDef.density = bc2d.Density;
				shapeDef.material.friction = bc2d.Friction;
				shapeDef.material.restitution = bc2d.Restitution;
				shapeDef.material.rollingResistance = bc2d.RestitutionThreshold;
				shapeDef.enableHitEvents = true;

				b2CreatePolygonShape(body, &shapeDef, &box);
			}

			if (entt.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entt.GetComponent<CircleCollider2DComponent>();

				b2ShapeDef shapeDef = b2DefaultShapeDef();
				shapeDef.density = cc2d.Density;
				shapeDef.material.friction = cc2d.Friction;
				shapeDef.material.restitution = cc2d.Restitution;
				shapeDef.material.rollingResistance = cc2d.RestitutionThreshold;
				shapeDef.enableHitEvents = true;

				b2Circle circle = { { cc2d.Offset.x, cc2d.Offset.y }, cc2d.Radius * scale.x };

				b2CreateCircleShape(body, &shapeDef, &circle);
			}
		});

		return true;
	}

	bool B2Physics::OnStop(Scene* p_Scene)
	{
		KTN_PROFILE_FUNCTION();

		auto& registry = p_Scene->GetRegistry();
		registry.view<TransformComponent, Rigidbody2DComponent>().each(
		[&](auto p_Entt, TransformComponent& p_Transform, Rigidbody2DComponent& p_RC)
		{
			if (p_RC.Body != B2BodyID{})
			{
				DestroyBody(p_RC.Body);
				p_RC.Body = {};
			}
		});

		return true;
	}

	bool B2Physics::OnInit()
	{
		KTN_PROFILE_FUNCTION();

		b2WorldDef worldDef = b2DefaultWorldDef();
		worldDef.gravity = { m_Gravity.x, m_Gravity.y };

		b2WorldId world = b2CreateWorld(&worldDef);
		m_World.Index = world.index1;
		m_World.Generation = world.generation;

		b2AABB bounds = { { -FLT_MAX, -FLT_MAX }, { FLT_MAX, FLT_MAX } };

		return true;
	}

	void B2Physics::OnUpdate(Scene* p_Scene)
	{
		KTN_PROFILE_FUNCTION();

		if (m_Paused) return;

		b2WorldId world = { .index1 = m_World.Index, .generation = m_World.Generation };

		auto ts = Time::GetDeltaTime();
		b2World_Step(world, (float)ts, 4);

		SyncTransforms(p_Scene);
	}

	void B2Physics::SetGravity(const glm::vec2& p_Gravity)
	{
		KTN_PROFILE_FUNCTION();

		if (m_Gravity == p_Gravity) return;

		b2WorldId world = { .index1 = m_World.Index, .generation = m_World.Generation };
		b2World_SetGravity(world, { p_Gravity.x, p_Gravity.y });
		m_Gravity = p_Gravity;
	}

	void B2Physics::SyncTransforms(Scene* p_Scene)
	{
		KTN_PROFILE_FUNCTION();

		if (m_Paused) return;

		auto& registry = p_Scene->GetRegistry();
		registry.view<TransformComponent, Rigidbody2DComponent>().each(
		[&](auto p_Entt, TransformComponent& p_Transform, Rigidbody2DComponent& p_RC)
		{
			Entity entt = { p_Entt, p_Scene };
			b2BodyId body = {
				.index1 = p_RC.Body.Index,
				.world0 = p_RC.Body.World,
				.generation = p_RC.Body.Generation
			};

			auto pos = b2Body_GetPosition(body);

			p_Transform.SetLocalTranslation({ pos.x, pos.y, p_Transform.GetLocalTranslation().z });
			p_Transform.SetLocalRotation({ p_Transform.GetLocalRotation().x, p_Transform.GetLocalRotation().y, b2Rot_GetAngle(b2Body_GetRotation(body)) });
			p_Transform.SetWorldMatrix(glm::mat4(1.0f));
		});
	}

} // namespace KTN
