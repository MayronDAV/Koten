#include "ktnpch.h"
#include "B2Physics.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/SystemManager.h"
#include "Koten/Scene/Entity.h"
#include "Koten/Script/ScriptEngine.h"

// lib
#include <box2d/box2d.h>

// std
#include <functional>



namespace std 
{
	template<>
	struct hash<std::pair<KTN::UUID, KTN::UUID>> 
	{
		size_t operator()(const std::pair<KTN::UUID, KTN::UUID>& p) const 
		{
			return hash<KTN::UUID>()(p.first) ^ (hash<KTN::UUID>()(p.second) << 1);
		}
	};

	template<>
	struct equal_to<std::pair<KTN::UUID, KTN::UUID>> 
	{
		bool operator()(const std::pair<KTN::UUID, KTN::UUID>& p_Lhs, const std::pair<KTN::UUID, KTN::UUID>& p_Rhs) const 
		{
			return p_Lhs.first == p_Rhs.first && p_Lhs.second == p_Rhs.second;
		}
	};

} // namespace std 

namespace KTN
{
	namespace
	{
		struct ShapeUserData
		{
			void* Component;
			Entity Entt;
			bool IsTrigger;
		};

		static std::unordered_map<AssetHandle, std::vector<ShapeUserData*>> s_ToDelete;
		static std::unordered_map<AssetHandle, std::unordered_set<std::pair<UUID, UUID>>> s_ActiveSensorEvents;
		static std::unordered_map<AssetHandle, std::unordered_set<std::pair<UUID, UUID>>> s_ActiveContactEvents;

		static ShapeUserData* GetShapeUserData(b2ShapeId p_ShapeId)
		{
			KTN_PROFILE_FUNCTION();

			if (!b2Shape_IsValid(p_ShapeId)) return nullptr;
			void* userData = b2Shape_GetUserData(p_ShapeId);
			return userData ? static_cast<ShapeUserData*>(userData) : nullptr;
		}

		static void SensorEvents(b2WorldId& p_World, Scene* p_Scene)
		{
			KTN_PROFILE_FUNCTION();

			auto sceneHandle = p_Scene->Handle;

			b2SensorEvents sensorEvents = b2World_GetSensorEvents(p_World);
			for (int i = 0; i < sensorEvents.beginCount; ++i)
			{
				b2SensorBeginTouchEvent* beginEvent = sensorEvents.beginEvents + i;

				auto* sensorData = GetShapeUserData(beginEvent->sensorShapeId);
				auto* visitorData = GetShapeUserData(beginEvent->visitorShapeId);

				if (!sensorData || !visitorData) continue;

				auto sensorID = sensorData->Entt.GetUUID();
				auto visitorID = visitorData->Entt.GetUUID();

				s_ActiveSensorEvents[sceneHandle].insert(std::make_pair(sensorID, visitorID));

				auto sensorInstance = ScriptEngine::GetEntityScriptInstance(sensorID);
				if (sensorInstance)
				{
					void* params[] = { &visitorID };
					sensorInstance->TryInvokeMethod("OnTriggerEnter", 1, params);
				}

				auto visitorInstance = ScriptEngine::GetEntityScriptInstance(visitorID);
				if (visitorInstance)
				{
					void* params[] = { &sensorID };
					visitorInstance->TryInvokeMethod("OnTriggerEnter", 1, params);
				}
			}

			for (int i = 0; i < sensorEvents.endCount; ++i)
			{
				b2SensorEndTouchEvent* endEvent = sensorEvents.endEvents + i;

				auto* sensorData = GetShapeUserData(endEvent->sensorShapeId);
				auto* visitorData = GetShapeUserData(endEvent->visitorShapeId);

				if (!sensorData || !visitorData) continue;

				auto sensorID = sensorData->Entt.GetUUID();
				auto visitorID = visitorData->Entt.GetUUID();

				s_ActiveSensorEvents[sceneHandle].erase(std::make_pair(sensorID, visitorID));
				s_ActiveSensorEvents[sceneHandle].erase(std::make_pair(visitorID, sensorID));

				auto sensorInstance = ScriptEngine::GetEntityScriptInstance(sensorID);
				if (sensorInstance)
				{
					void* params[] = { &visitorID };
					sensorInstance->TryInvokeMethod("OnTriggerExit", 1, params);
				}

				auto visitorInstance = ScriptEngine::GetEntityScriptInstance(visitorID);
				if (visitorInstance)
				{
					void* params[] = { &sensorID };
					visitorInstance->TryInvokeParentMethod("OnTriggerExit", 1, params);
				}
			}

			for (const auto& collisionPair : s_ActiveSensorEvents[sceneHandle])
			{
				auto sensorID = collisionPair.first;
				auto visitorID = collisionPair.second;

				auto sensorInstance = ScriptEngine::GetEntityScriptInstance(sensorID);
				if (sensorInstance)
				{
					void* params[] = { &visitorID };
					sensorInstance->TryInvokeMethod("OnTriggerStay", 1, params);
				}

				auto visitorInstance = ScriptEngine::GetEntityScriptInstance(visitorID);
				if (visitorInstance)
				{
					void* params[] = { &sensorID };
					visitorInstance->TryInvokeMethod("OnTriggerStay", 1, params);
				}
			}
		}

		static void ContactEvents(b2WorldId& p_World, Scene* p_Scene)
		{
			KTN_PROFILE_FUNCTION();

			auto sceneHandle = p_Scene->Handle;

			b2ContactEvents contactEvents = b2World_GetContactEvents(p_World);
			for (int i = 0; i < contactEvents.beginCount; ++i)
			{
				b2ContactBeginTouchEvent* beginEvent = contactEvents.beginEvents + i;

				auto* userDataA = GetShapeUserData(beginEvent->shapeIdA);
				auto* userDataB = GetShapeUserData(beginEvent->shapeIdB);

				if (!userDataA || !userDataB) continue;

				auto aID = userDataA->Entt.GetUUID();
				auto bID = userDataB->Entt.GetUUID();

				s_ActiveContactEvents[sceneHandle].insert(std::make_pair(aID, bID));

				auto aInstance = ScriptEngine::GetEntityScriptInstance(aID);
				if (aInstance)
				{
					void* params[] = { &bID };
					aInstance->TryInvokeMethod("OnCollisionEnter", 1, params);
				}

				auto bInstance = ScriptEngine::GetEntityScriptInstance(bID);
				if (bInstance)
				{
					void* params[] = { &aID };
					bInstance->TryInvokeMethod("OnCollisionEnter", 1, params);
				}
			}

			for (int i = 0; i < contactEvents.endCount; ++i)
			{
				b2ContactEndTouchEvent* endEvent = contactEvents.endEvents + i;

				auto* userDataA = GetShapeUserData(endEvent->shapeIdA);
				auto* userDataB = GetShapeUserData(endEvent->shapeIdB);

				if (!userDataA || !userDataB) continue;

				auto aID = userDataA->Entt.GetUUID();
				auto bID = userDataB->Entt.GetUUID();

				s_ActiveContactEvents[sceneHandle].insert(std::make_pair(aID, bID));

				auto aInstance = ScriptEngine::GetEntityScriptInstance(aID);
				if (aInstance)
				{
					void* params[] = { &bID };
					aInstance->TryInvokeMethod("OnCollisionExit", 1, params);
				}

				auto bInstance = ScriptEngine::GetEntityScriptInstance(bID);
				if (bInstance)
				{
					void* params[] = { &aID };
					bInstance->TryInvokeMethod("OnCollisionExit", 1, params);
				}
			}

			for (const auto& collisionPair : s_ActiveContactEvents[sceneHandle])
			{
				auto aID = collisionPair.first;
				auto bID = collisionPair.second;

				s_ActiveContactEvents[sceneHandle].insert(std::make_pair(aID, bID));

				auto aInstance = ScriptEngine::GetEntityScriptInstance(aID);
				if (aInstance)
				{
					void* params[] = { &bID };
					aInstance->TryInvokeMethod("OnCollisionStay", 1, params);
				}

				auto bInstance = ScriptEngine::GetEntityScriptInstance(bID);
				if (bInstance)
				{
					void* params[] = { &aID };
					bInstance->TryInvokeMethod("OnCollisionStay", 1, params);
				}
			}
		}

	} // namespace

	B2Physics::B2Physics()
	{
		KTN_PROFILE_FUNCTION();
	}

	B2Physics::~B2Physics()
	{
		KTN_PROFILE_FUNCTION();

		if (!s_ToDelete.empty())
		{
			for (auto& [scene, toDelete] : s_ToDelete)
			{
				for (auto& userData : toDelete)
				{
					if (userData)
						delete userData;
				}
			}
			s_ToDelete.clear();
		}

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

		auto sceneHandle = p_Scene->Handle;

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

				auto* userData = new ShapeUserData{
					.Component = &bc2d,
					.Entt = entt,
					.IsTrigger = bc2d.IsTrigger
				};
				s_ToDelete[sceneHandle].push_back(userData);

				b2ShapeDef shapeDef = b2DefaultShapeDef();
				shapeDef.density = bc2d.Density;
				shapeDef.material.friction = bc2d.Friction;
				shapeDef.material.restitution = bc2d.Restitution;
				shapeDef.material.rollingResistance = bc2d.RestitutionThreshold;
				shapeDef.userData = userData;
				shapeDef.enableSensorEvents = true;
				shapeDef.enableContactEvents = true;
				if (bc2d.IsTrigger)
				{
					shapeDef.isSensor = true;
					shapeDef.density = 0.0f;
				}

				b2CreatePolygonShape(body, &shapeDef, &box);
			}

			if (entt.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entt.GetComponent<CircleCollider2DComponent>();

				auto* userData = new ShapeUserData{
					.Component = &cc2d,
					.Entt = entt,
					.IsTrigger = cc2d.IsTrigger
				};
				s_ToDelete[sceneHandle].push_back(userData);

				b2ShapeDef shapeDef = b2DefaultShapeDef();
				shapeDef.density = cc2d.Density;
				shapeDef.material.friction = cc2d.Friction;
				shapeDef.material.restitution = cc2d.Restitution;
				shapeDef.material.rollingResistance = cc2d.RestitutionThreshold;
				shapeDef.userData = userData;
				shapeDef.enableSensorEvents = true;
				shapeDef.enableContactEvents = true;
				if (cc2d.IsTrigger)
				{
					shapeDef.isSensor = true;
					shapeDef.density = 0.0f;
				}

				b2Circle circle = { { cc2d.Offset.x, cc2d.Offset.y }, cc2d.Radius * scale.x };

				b2CreateCircleShape(body, &shapeDef, &circle);
			}
		});

		return true;
	}

	bool B2Physics::OnStop(Scene* p_Scene)
	{
		KTN_PROFILE_FUNCTION();

		auto sceneHandle = p_Scene->Handle;
		for (ShapeUserData* userData : s_ToDelete[sceneHandle])
		{
			if (userData)
				delete userData;
		}
		s_ToDelete.erase(sceneHandle);
		s_ActiveSensorEvents.erase(sceneHandle);
		s_ActiveContactEvents.erase(sceneHandle);

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
		worldDef.userData = this;

		b2WorldId world = b2CreateWorld(&worldDef);
		m_World.Index = world.index1;
		m_World.Generation = world.generation;

		return true;
	}

	void B2Physics::OnUpdate(Scene* p_Scene)
	{
		KTN_PROFILE_FUNCTION();

		if (m_Paused || p_Scene->IsPaused()) return;

		b2WorldId world = { .index1 = m_World.Index, .generation = m_World.Generation };

		auto ts = Time::GetDeltaTime();
		b2World_Step(world, (float)ts, 4);

		SyncTransforms(p_Scene);

		SensorEvents(world, p_Scene);

		ContactEvents(world, p_Scene);
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
