#include "ktnpch.h"
#include "Collider2DSystem.h"
#include "Koten/Collision/AABBTree.h"
#include "Koten/Collision/Intersect2D.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/Entity.h"
#include "Koten/Script/ScriptEngine.h"
#include "Koten/Asset/AssetManager.h"

// lib
#include <box2d/box2d.h>




namespace KTN
{
	namespace
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

			KTN_CORE_ERROR("Unsupported shape for AABB");
			return { };
		}

		static BoxShape ComputeBox(const TransformComponent& p_Transform, const Collider2DComponent& p_Collider)
		{
			KTN_PROFILE_FUNCTION_LOW();

			if (p_Collider.Shape != Collider2DShape::Box)
			{
				KTN_CORE_ERROR("The Shape should be Collider2DShape::Box");
				return {};
			}

			BoxShape box{};
			glm::vec2 scale = p_Transform.GetWorldScale();
			glm::vec2 half = (p_Collider.Size * 0.5f) * scale;

			box.Center = glm::vec2(p_Transform.GetWorldTranslation()) + p_Collider.Offset * scale;
			box.HalfExtents = half;
			box.Rotation = p_Transform.GetWorldRotation().z;

			return box;
		}

		static CircleShape ComputeCircle(const TransformComponent& p_Transform, const Collider2DComponent& p_Collider)
		{
			KTN_PROFILE_FUNCTION_LOW();

			if (p_Collider.Shape != Collider2DShape::Circle)
			{
				KTN_CORE_ERROR("The Shape should be Collider2DShape::Circle");
				return {};
			}

			CircleShape c{};
			glm::vec2 scale = p_Transform.GetWorldScale();
			float maxScale = glm::max(scale.x, scale.y);

			c.Center = glm::vec2(p_Transform.GetWorldTranslation()) + p_Collider.Offset * scale;
			c.Radius = p_Collider.Size.x * maxScale;

			return c;
		}
	
	} // namespace

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
		m_CachedShape.clear();
		m_ContactsInfo.clear();
		m_StayContacts.clear();
		m_BeginContacts.clear();
		m_EndContacts.clear();

		return true;
	}

	bool Collider2DSystem::OnInit()
	{
		return true;
	}

	void Collider2DSystem::OnUpdate(Scene* p_Scene)
	{
		KTN_PROFILE_FUNCTION();
		
		auto pairs = m_BroadPhase->ComputePairs();

		m_ContactsInfo.reserve(pairs.size());
		m_CachedShape.reserve(pairs.size() * 2);

		m_BeginContacts.clear();
		m_EndContacts.clear();
		m_BeginContacts.reserve(pairs.size());
		m_EndContacts.reserve(pairs.size());
		m_StayContacts.reserve(pairs.size());

		for (auto& [idA, idB] : pairs)
		{
			Entity A = p_Scene->GetEntityByUUID(idA);
			Entity B = p_Scene->GetEntityByUUID(idB);

			if (A.HasComponent<Rigidbody2DComponent>() && B.HasComponent<Rigidbody2DComponent>()) continue;

			auto& colA = A.GetComponent<Collider2DComponent>();
			auto& colB = B.GetComponent<Collider2DComponent>();

			auto& trA = A.GetComponent<TransformComponent>();
			auto& trB = B.GetComponent<TransformComponent>();

			size_t seed = 0;
			{
				UUID a = idA;
				UUID b = idB;
				HashCombine(seed, a, b);
			}

			if ( m_CachedShape.find(idA) == m_CachedShape.end() ||
				(m_CachedShape.find(idA) != m_CachedShape.end() && m_CachedShape[idA].Position != trA.GetWorldTranslation()))
			{
				auto& shape = m_CachedShape[idA];

				shape.Type = colA.Shape;
				if (colA.Shape == Collider2DShape::Box)
					shape.Box = ComputeBox(trA, colA);
				else if (colA.Shape == Collider2DShape::Circle)
					shape.Circle = ComputeCircle(trA, colA);
				else
				{
					KTN_CORE_ERROR("Unsupported cached shape!");
					shape.Type = Collider2DShape::Box;
					shape.Box = {};
				}

				shape.Position = trA.GetWorldTranslation();
			}

			if ( m_CachedShape.find(idB) == m_CachedShape.end() ||
				(m_CachedShape.find(idB) != m_CachedShape.end() && m_CachedShape[idB].Position != trB.GetWorldTranslation()))
			{
				auto& shape = m_CachedShape[idB];

				shape.Type = colB.Shape;
				if (colB.Shape == Collider2DShape::Box)
					shape.Box = ComputeBox(trB, colB);
				else if (colB.Shape == Collider2DShape::Circle)
					shape.Circle = ComputeCircle(trB, colB);
				else
				{
					KTN_CORE_ERROR("Unsupported cached shape!");
					shape.Type = Collider2DShape::Box;
					shape.Box = {};
				}

				shape.Position = trB.GetWorldTranslation();
			}

			bool overlap = false;
			if (colA.Shape == Collider2DShape::Box && colB.Shape == Collider2DShape::Box)
				overlap = Intersect2D::IsColliding(m_CachedShape[idA].Box, m_CachedShape[idB].Box);
			else if (colA.Shape == Collider2DShape::Circle &&colB.Shape == Collider2DShape::Circle)
				overlap = Intersect2D::IsColliding(m_CachedShape[idA].Circle, m_CachedShape[idB].Circle);
			else if ((colA.Shape == Collider2DShape::Circle || colB.Shape == Collider2DShape::Circle) &&
					 (colA.Shape == Collider2DShape::Box    || colB.Shape == Collider2DShape::Box))
			{
				overlap = Intersect2D::IsColliding(
					colA.Shape == Collider2DShape::Circle ? m_CachedShape[idA].Circle : m_CachedShape[idB].Circle,
					colA.Shape == Collider2DShape::Box    ? m_CachedShape[idA].Box : m_CachedShape[idB].Box);
			}

			auto it = m_ContactsInfo.find(seed);
			bool exists = it != m_ContactsInfo.end();
			if (overlap)
			{
				if (exists)
				{
					auto& info = m_ContactsInfo[seed];

					if (info.Flags & ContactFlagsStoppedTouching)
						info.Flags &= ~ContactFlagsStoppedTouching;

					if (info.Flags & ContactFlagsStartedTouching)
					{
						info.Flags &= ~ContactFlagsStartedTouching;
						info.Flags |= ContactFlagsTouching;
						m_StayContacts.push_back(info.Event);
						info.Index = (int)m_StayContacts.size() - 1;
					}
				}
				else
				{
					Contact contact{};
					contact.Event.IdA = idA;
					contact.Event.IdB = idB;
					contact.Flags |= ContactFlagsStartedTouching;
					m_BeginContacts.push_back(contact.Event);
					contact.Index = (int)m_BeginContacts.size() - 1;
					m_ContactsInfo[seed] = contact;
				}
			}
			else if (exists)
			{
				auto& info = m_ContactsInfo[seed];

				if (info.Flags & ContactFlagsStartedTouching)
				{
					info.Flags &= ~ContactFlagsStartedTouching;

					if (info.Index != -1)
					{
						m_BeginContacts.erase(m_BeginContacts.begin() + (size_t)info.Index);
						info.Index = -1;
					}
				}

				if (info.Flags & ContactFlagsTouching)
				{
					info.Flags &= ~ContactFlagsTouching;

					if (info.Index != -1)
					{
						m_StayContacts.erase(m_StayContacts.begin() + (size_t)info.Index);
						info.Index = -1;
					}
				}

				if (!(info.Flags & ContactFlagsStoppedTouching))
					info.Flags |= ContactFlagsStoppedTouching;

				m_EndContacts.push_back(info.Event);
				m_ContactsInfo.erase(seed);
			}
		}

		for (const auto& event : m_BeginContacts)
		{
			Entity A = p_Scene->GetEntityByUUID(event.IdA);
			Entity B = p_Scene->GetEntityByUUID(event.IdB);

			auto& trA = A.GetComponent<TransformComponent>();
			auto& trB = B.GetComponent<TransformComponent>();
			auto posA = trA.GetWorldTranslation();
			auto posB = trB.GetWorldTranslation();

			KTN_CORE_INFO("OnCollisionEnter {} {}", (uint64_t)event.IdA, (uint64_t)event.IdB);
			KTN_CORE_INFO("    A: ({}, {}), B: ({}, {})", posA.x, posA.y, posB.x, posB.y);
		}

		for (const auto& event : m_StayContacts)
		{
			Entity A = p_Scene->GetEntityByUUID(event.IdA);
			Entity B = p_Scene->GetEntityByUUID(event.IdB);

			auto& trA = A.GetComponent<TransformComponent>();
			auto& trB = B.GetComponent<TransformComponent>();
			auto posA = trA.GetWorldTranslation();
			auto posB = trB.GetWorldTranslation();

			KTN_CORE_INFO("OnCollisionStay {} {}", (uint64_t)event.IdA, (uint64_t)event.IdB);
			KTN_CORE_INFO("    A: ({}, {}), B: ({}, {})", posA.x, posA.y, posB.x, posB.y);
		}

		for (const auto& event : m_EndContacts)
		{
			Entity A = p_Scene->GetEntityByUUID(event.IdA);
			Entity B = p_Scene->GetEntityByUUID(event.IdB);

			auto& trA = A.GetComponent<TransformComponent>();
			auto& trB = B.GetComponent<TransformComponent>();
			auto posA = trA.GetWorldTranslation();
			auto posB = trB.GetWorldTranslation();

			KTN_CORE_INFO("OnCollisionEnd {} {}", (uint64_t)event.IdA, (uint64_t)event.IdB);
			KTN_CORE_INFO("    A: ({}, {}), B: ({}, {})", posA.x, posA.y, posB.x, posB.y);
		}
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