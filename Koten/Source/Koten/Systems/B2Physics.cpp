#include "ktnpch.h"
#include "B2Physics.h"
#include "Koten/Scene/Scene.h"
#include "Koten/Scene/SystemManager.h"
#include "Koten/Script/ScriptEngine.h"
#include "Koten/Asset/AssetManager.h"

// lib
#include <box2d/box2d.h>

// std
#include <functional>



namespace KTN
{
    namespace
    {
        enum Category : uint8_t
        {
            CategoryStatic = BIT(0),
            CategoryCharacter = BIT(1),
            CategoryDynamic = BIT(2),
            CategoryTrigger = BIT(3)
        };

        static b2WorldId GetWorldID(const B2WorldID& p_World)
        {
            KTN_PROFILE_FUNCTION();
            b2WorldId worldId = {};
            worldId.index1 = p_World.Index;
            worldId.generation = p_World.Generation;
            return worldId;
        }

        static b2BodyId GetBodyID(const B2BodyID& p_Body)
        {
            KTN_PROFILE_FUNCTION();
            b2BodyId bodyId = {};
            bodyId.index1 = p_Body.Index;
            bodyId.world0 = p_Body.World;
            bodyId.generation = p_Body.Generation;
            return bodyId;
        }

        static B2Physics::ShapeUserData* GetShapeUserData(b2ShapeId p_ShapeId)
        {
            KTN_PROFILE_FUNCTION();

            if (!b2Shape_IsValid(p_ShapeId)) return nullptr;
            void* userData = b2Shape_GetUserData(p_ShapeId);
            return userData ? static_cast<B2Physics::ShapeUserData*>(userData) : nullptr;
        }

        static uint64_t GetFilterCategory(PhysicsBody2D* p_Body)
        {
            KTN_PROFILE_FUNCTION();
            if (p_Body->IsTrigger)                                          return CategoryTrigger;
            if (p_Body->GetType() == Rigidbody2DComponent::Type::Static)    return CategoryStatic;
            if (p_Body->GetType() == Rigidbody2DComponent::Type::Character) return CategoryCharacter;
            if (p_Body->GetType() == Rigidbody2DComponent::Type::Rigidbody) return CategoryDynamic;
            return B2_DEFAULT_CATEGORY_BITS;
        }

        struct Hit
        {
            b2ShapeId Shape;
            b2Manifold Manifold;
        };

        struct Context
        {
            Entity CharacterEntity;
            std::vector<Hit> Collisions;
            b2Transform CharacterTransform = {};
            b2ShapeId CharacterShape = {};
            b2BodyId CharacterBody = {};
            glm::vec2 UpDirection = { 0.0f, 1.0f };
            float FloorMaxAngle = glm::radians(45.0f);
            float FloorMinCos = 0.0f;
            bool IsMoveAndSlide = false;
            bool SlideOnCeiling = true;
            float WallMinSlideAngle = glm::radians(15.0f);

            Context(Entity p_Entity, bool p_IsSlide)
                : CharacterEntity(p_Entity), IsMoveAndSlide(p_IsSlide)
            {
                if (p_Entity.HasComponent<CharacterBody2DComponent>())
                {
                    auto& character = p_Entity.GetComponent<CharacterBody2DComponent>();
                    UpDirection = character.UpDirection;
                    FloorMaxAngle = character.FloorMaxAngle;
                    SlideOnCeiling = character.SlideOnCeiling;
                    WallMinSlideAngle = character.WallMinSlideAngle;
                    FloorMinCos = cosf(FloorMaxAngle);
                }
            }
        };

        static bool OverlapCallback(b2ShapeId p_ShapeId, void* p_Context)
        {
            KTN_PROFILE_FUNCTION();

            auto* ctx = static_cast<Context*>(p_Context);

            if (ctx->CharacterShape.index1 == p_ShapeId.index1)
                return true;

            auto cShapeType = b2Shape_GetType(ctx->CharacterShape);
            auto shapeType = b2Shape_GetType(p_ShapeId);

            auto body = b2Shape_GetBody(p_ShapeId);
            if (body.index1 == ctx->CharacterBody.index1)
                return true;

            auto trB = b2Body_GetTransform(body);

            b2Manifold manifold = {};
            if (cShapeType == b2_polygonShape && shapeType == b2_polygonShape)
            {
                auto sA = b2Shape_GetPolygon(ctx->CharacterShape);
                auto sB = b2Shape_GetPolygon(p_ShapeId);
                manifold = b2CollidePolygons(&sA, ctx->CharacterTransform, &sB, trB);
            }
            else if (cShapeType == b2_circleShape && shapeType == b2_circleShape)
            {
                auto sA = b2Shape_GetCircle(ctx->CharacterShape);
                auto sB = b2Shape_GetCircle(p_ShapeId);
                manifold = b2CollideCircles(&sA, ctx->CharacterTransform, &sB, trB);
            }
            else if ((cShapeType == b2_polygonShape || shapeType == b2_polygonShape) && 
                     (cShapeType == b2_circleShape || shapeType == b2_circleShape))
            {
                auto sA = cShapeType == b2_polygonShape  ? b2Shape_GetPolygon(ctx->CharacterShape) : b2Shape_GetPolygon(p_ShapeId);
                auto sB = cShapeType == b2_circleShape ? b2Shape_GetCircle(ctx->CharacterShape) : b2Shape_GetCircle(p_ShapeId);
                manifold = b2CollidePolygonAndCircle(&sA, ctx->CharacterTransform, &sB, trB);
            }

            if (manifold.pointCount > 0)
            {
                ctx->Collisions.push_back({ p_ShapeId, manifold });
            }

            return true;
        }

        static std::pair<UUID, UUID> MakePair(const UUID& p_A, const UUID& p_B)
        {
            return p_A < p_B ? std::make_pair(p_A, p_B) : std::make_pair(p_B, p_A);
        }

        static B2BodyID GetPhysicsBody2D(Entity p_Entity)
        {
            KTN_PROFILE_FUNCTION_LOW();

            if (p_Entity.HasComponent<Rigidbody2DComponent>())
                return p_Entity.GetComponent<Rigidbody2DComponent>().Body;
            else if (p_Entity.HasComponent<CharacterBody2DComponent>())
                return p_Entity.GetComponent<CharacterBody2DComponent>().Body;
            else if (p_Entity.HasComponent<StaticBody2DComponent>())
                return p_Entity.GetComponent<StaticBody2DComponent>().Body;

            return {};
        }


    } // namespace

    B2Physics::B2Physics()
    {
        KTN_PROFILE_FUNCTION();
    }

    B2Physics::~B2Physics()
    {
        KTN_PROFILE_FUNCTION();

        for (auto* userData : m_ToDelete)
        {
            delete userData;
        }
        m_ToDelete.clear();

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

        registry.view<TransformComponent, CharacterBody2DComponent>().each(
        [&](auto p_Entt, TransformComponent& p_Transform, CharacterBody2DComponent& p_Body)
        {
            Entity entt{ p_Entt, p_Scene };
            if (!entt.IsActive()) return;

            CreatePhysicsBody(p_Scene, entt, p_Transform, &p_Body);
        });

        registry.view<TransformComponent, Rigidbody2DComponent>().each(
        [&](auto p_Entt, TransformComponent& p_Transform, Rigidbody2DComponent& p_Body)
        {
            Entity entt{ p_Entt, p_Scene };
            if (!entt.IsActive()) return;

            CreatePhysicsBody(p_Scene, entt, p_Transform, &p_Body);
        });

        registry.view<TransformComponent, StaticBody2DComponent>().each(
        [&](auto p_Entt, TransformComponent& p_Transform, StaticBody2DComponent& p_Body)
        {
            Entity entt{ p_Entt, p_Scene };
            if (!entt.IsActive()) return;

            CreatePhysicsBody(p_Scene, entt, p_Transform, &p_Body);
        });

        m_IsRunning = true;
        return true;
    }

    bool B2Physics::OnStop(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

        auto sceneHandle = p_Scene->Handle;
        for (ShapeUserData* userData : m_ToDelete)
        {
            delete userData;
        }
        m_ToDelete.clear();
        m_ActiveSensorEvents.clear();
        m_ActiveContactEvents.clear();
        m_FrameContactEvents.clear();
        m_ActiveFrameContactEvents.clear();

        auto& registry = p_Scene->GetRegistry();
        registry.view<TransformComponent, PhysicsBody2D>().each(
        [&](auto p_Entt, TransformComponent& p_Transform, PhysicsBody2D& p_Body)
        {
            if (p_Body.Body != B2BodyID{})
            {
                DestroyBody(p_Body.Body);
                p_Body.Body = {};
            }
        });

        m_IsRunning = false;
        return true;
    }

    void B2Physics::OnCreateEntity(Entity p_Entity)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Entity.IsActive()) return;

        if (!p_Entity.HasComponent<TransformComponent>()) return;

        auto& transform = p_Entity.GetComponent<TransformComponent>();

        if (p_Entity.HasComponent<CharacterBody2DComponent>())
        {
            auto& body = p_Entity.GetComponent<CharacterBody2DComponent>();
            body.Body = {};
            CreatePhysicsBody(p_Entity.GetScene(), p_Entity, transform, &body);
        }
        else if (p_Entity.HasComponent<Rigidbody2DComponent>())
        {
            auto& body = p_Entity.GetComponent<Rigidbody2DComponent>();
            body.Body = {};
            CreatePhysicsBody(p_Entity.GetScene(), p_Entity, transform, &body);
        }
        else if (p_Entity.HasComponent<StaticBody2DComponent>())
        {
            auto& body = p_Entity.GetComponent<StaticBody2DComponent>();
            body.Body = {};
            CreatePhysicsBody(p_Entity.GetScene(), p_Entity, transform, &body);
        }
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

        auto ts = Time::GetDeltaTime();
        b2World_Step(GetWorldID(m_World), (float)ts, 4);

        SyncTransforms(p_Scene);

        SensorEvents(p_Scene);

        ContactEvents(p_Scene);
    }

    glm::vec2 B2Physics::GetGravity(Entity p_Entity) const
    {
        auto gravity = m_Gravity;
        auto rigid = p_Entity.TryGetComponent<Rigidbody2DComponent>();
        if (rigid)
        {
            gravity *= rigid->GravityScale;
        }

        return gravity;
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

        // TODO: Do not duplicate the code.

        auto& registry = p_Scene->GetRegistry();
        registry.view<TransformComponent, CharacterBody2DComponent>().each(
        [&](auto p_Entt, TransformComponent& p_Transform, CharacterBody2DComponent& p_Body)
        {
            if (p_Body.Body == B2BodyID{}) return;

            Entity entt = { p_Entt, p_Scene };
            b2BodyId body = {
                .index1 = p_Body.Body.Index,
                .world0 = p_Body.Body.World,
                .generation = p_Body.Body.Generation
            };

            auto pos = b2Body_GetPosition(body);
            p_Transform.SetLocalTranslation({ pos.x, pos.y, p_Transform.GetLocalTranslation().z });
            p_Transform.SetLocalRotation({ p_Transform.GetLocalRotation().x, p_Transform.GetLocalRotation().y, b2Rot_GetAngle(b2Body_GetRotation(body)) });
            p_Transform.SetWorldMatrix(glm::mat4(1.0f));
        });

        registry.view<TransformComponent, Rigidbody2DComponent>().each(
        [&](auto p_Entt, TransformComponent& p_Transform, Rigidbody2DComponent& p_Body)
        {
            if (p_Body.Body == B2BodyID{}) return;

            Entity entt = { p_Entt, p_Scene };
            b2BodyId body = {
                .index1 = p_Body.Body.Index,
                .world0 = p_Body.Body.World,
                .generation = p_Body.Body.Generation
            };

            auto pos = b2Body_GetPosition(body);
            p_Transform.SetLocalTranslation({ pos.x, pos.y, p_Transform.GetLocalTranslation().z });
            p_Transform.SetLocalRotation({ p_Transform.GetLocalRotation().x, p_Transform.GetLocalRotation().y, b2Rot_GetAngle(b2Body_GetRotation(body)) });
            p_Transform.SetWorldMatrix(glm::mat4(1.0f));
        });
    }

    void B2Physics::SetTransform(Entity p_Entity, const glm::vec3& p_Pos, float p_Rot)
    {
        KTN_PROFILE_FUNCTION();

        auto body = GetPhysicsBody2D(p_Entity);

        if (body == B2BodyID{}) return;

        b2BodyId bodyId = {
            .index1 = body.Index,
            .world0 = body.World,
            .generation = body.Generation
        };

        b2Body_SetTransform(bodyId, { p_Pos.x, p_Pos.z }, b2MakeRot(p_Rot));
    }

    void B2Physics::MoveAndSlide(Entity p_Entity)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Entity.HasComponent<CharacterBody2DComponent>())
            return;

        auto& character = p_Entity.GetComponent<CharacterBody2DComponent>();
        if (character.Body == B2BodyID{})
            return;

        auto it = std::erase_if(m_FrameContactEvents, [&](const std::pair<UUID, UUID>& pair) -> bool
        {
            auto id = p_Entity.GetUUID();
            return id == pair.first || id == pair.second;
        });

        b2BodyId body = {
            character.Body.Index,
            character.Body.World,
            character.Body.Generation
        };


        b2Vec2 velocity = b2Body_GetLinearVelocity(body);
        if (b2Length(velocity) == 0.0f && (!character.OnFloor && !character.OnCeiling && !character.OnWall))
            return;

        b2ShapeId shape;
        b2Body_GetShapes(body, &shape, 1);

        b2Transform start = b2Body_GetTransform(body);

        Context context(p_Entity, true);
        context.CharacterShape = shape;
        context.CharacterBody = body;
        context.CharacterTransform = start;

        b2QueryFilter filter{};
        filter.categoryBits = CategoryCharacter;
        filter.maskBits = CategoryStatic | CategoryCharacter;

        auto aabb = b2Body_ComputeAABB(body);
        b2World_OverlapAABB({ m_World.Index, m_World.Generation }, aabb, filter, OverlapCallback, &context);

        character.OnFloor = character.OnWall = character.OnCeiling = false;

        for (auto& collision : context.Collisions)
        {
            auto& manifold = collision.Manifold;

            bool changed = false;
            for (auto& point : manifold.points)
            {
                if (point.separation > 0.001f)
                    continue;

                float safeFraction = point.separation + 0.001f;
                start.p = start.p + (safeFraction * manifold.normal);
                changed = true;
            }

            if (changed)
                b2Body_SetTransform(body, start.p, b2Body_GetRotation(body));

            float dotUp = b2Dot(-manifold.normal, { character.UpDirection.x, character.UpDirection.y });
            if (dotUp > context.FloorMinCos)       character.OnFloor = true;
            else if (dotUp < -context.FloorMinCos) character.OnCeiling = true;
            else                                   character.OnWall = true;

            float vn = b2Dot(velocity, -manifold.normal);
            if (vn < 0.0f)
            {
                b2Body_SetLinearVelocity(body, { manifold.normal.x != 0.0f ? 0.0f : velocity.x, manifold.normal.y != 0.0f ? 0.0f : velocity.y });
            }

            auto* userdata = GetShapeUserData(collision.Shape);
            KTN_CORE_ASSERT(userdata, "ShapeUserData is null!");
            auto uuid = p_Entity.GetScene()->GetRegistry().get<IDComponent>(userdata->ID).ID;
            auto pair = MakePair(p_Entity.GetUUID(), uuid);
            if (!m_FrameContactEvents.contains(pair))
                m_FrameContactEvents.insert(pair);
        }

        OverlapContactEvents(p_Entity.GetScene());
    }

    void B2Physics::MoveAndCollide(Entity p_Entity)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Entity.HasComponent<CharacterBody2DComponent>())
            return;

        auto& character = p_Entity.GetComponent<CharacterBody2DComponent>();
        if (character.Body == B2BodyID{})
            return;

        auto it = std::erase_if(m_FrameContactEvents, [&](const std::pair<UUID, UUID>& pair) -> bool
        {
            auto id = p_Entity.GetUUID();
            return id == pair.first || id == pair.second;
        });

        b2BodyId body = {
            character.Body.Index,
            character.Body.World,
            character.Body.Generation
        };


        b2Vec2 velocity = b2Body_GetLinearVelocity(body);
        if (b2Length(velocity) == 0.0f && (!character.OnFloor && !character.OnCeiling && !character.OnWall))
            return;

        b2ShapeId shape;
        b2Body_GetShapes(body, &shape, 1);

        b2Transform start = b2Body_GetTransform(body);

        Context context(p_Entity, false);
        context.CharacterShape = shape;
        context.CharacterBody = body;
        context.CharacterTransform = start;

        b2QueryFilter filter{};
        filter.categoryBits = CategoryCharacter;
        filter.maskBits = CategoryStatic | CategoryCharacter;

        auto aabb = b2Body_ComputeAABB(body);
        b2World_OverlapAABB({ m_World.Index, m_World.Generation }, aabb, filter, OverlapCallback, &context);

        character.OnFloor = character.OnWall = character.OnCeiling = false;

        for (auto& collision : context.Collisions)
        {
            auto& manifold = collision.Manifold;

            bool changed = false;
            for (auto& point : manifold.points)
            {
                if (point.separation > 0.001f)
                    continue;

                float safeFraction = point.separation + 0.001f;
                start.p = start.p + (safeFraction * manifold.normal);
                changed = true;
            }

            if (changed)
                b2Body_SetTransform(body, start.p, b2Body_GetRotation(body));

            float dotUp = b2Dot(-manifold.normal, { character.UpDirection.x, character.UpDirection.y });
            if (dotUp > context.FloorMinCos)       character.OnFloor = true;
            else if (dotUp < -context.FloorMinCos) character.OnCeiling = true;
            else                                   character.OnWall = true;

            float vn = b2Dot(velocity, -manifold.normal);
            if (vn < 0.0f)
            {
                b2Body_SetLinearVelocity(body, b2Vec2_zero);
            }

            auto* userdata = GetShapeUserData(collision.Shape);
            KTN_CORE_ASSERT(userdata, "ShapeUserData is null!");
            auto uuid = p_Entity.GetScene()->GetRegistry().get<IDComponent>(userdata->ID).ID;
            auto pair = MakePair(p_Entity.GetUUID(), uuid);
            if (!m_FrameContactEvents.contains(pair))
                m_FrameContactEvents.insert(pair);
        }

        OverlapContactEvents(p_Entity.GetScene());
    }

    void B2Physics::SensorEvents(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

        b2WorldId world = GetWorldID(m_World);

        auto sceneHandle = p_Scene->Handle;
        auto& registry = p_Scene->GetRegistry();

        b2SensorEvents sensorEvents = b2World_GetSensorEvents(world);
        for (int i = 0; i < sensorEvents.beginCount; ++i)
        {
            b2SensorBeginTouchEvent* beginEvent = sensorEvents.beginEvents + i;

            auto* sensorData = GetShapeUserData(beginEvent->sensorShapeId);
            auto* visitorData = GetShapeUserData(beginEvent->visitorShapeId);

            if (!sensorData || !visitorData) continue;

            if (!registry.valid(sensorData->ID) || !registry.valid(visitorData->ID))
            {
                KTN_CORE_ERROR("One of the entities involved in the sensor begin event is invalid. Sensor ID: '{}', Visitor ID: '{}'", (uint32_t)sensorData->ID, (uint32_t)visitorData->ID);
                continue;
            }

            auto sensorID = registry.get<IDComponent>(sensorData->ID).ID;
            auto visitorID = registry.get<IDComponent>(visitorData->ID).ID;

            m_ActiveSensorEvents.insert(std::make_pair(sensorID, visitorID));

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

            if (!registry.valid(sensorData->ID) || !registry.valid(visitorData->ID))
            {
                KTN_CORE_ERROR("One of the entities involved in the sensor end event is invalid. Sensor ID: '{}', Visitor ID: '{}'", (uint32_t)sensorData->ID, (uint32_t)visitorData->ID);
                continue;
            }

            auto sensorID = registry.get<IDComponent>(sensorData->ID).ID;
            auto visitorID = registry.get<IDComponent>(visitorData->ID).ID;

            m_ActiveSensorEvents.erase(std::make_pair(sensorID, visitorID));
            m_ActiveSensorEvents.erase(std::make_pair(visitorID, sensorID));

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

        for (const auto& collisionPair : m_ActiveSensorEvents)
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

    void B2Physics::ContactEvents(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

        b2WorldId world = GetWorldID(m_World);

        auto sceneHandle = p_Scene->Handle;
        auto& registry = p_Scene->GetRegistry();

        b2ContactEvents contactEvents = b2World_GetContactEvents(world);
        for (int i = 0; i < contactEvents.beginCount; ++i)
        {
            b2ContactBeginTouchEvent* beginEvent = contactEvents.beginEvents + i;

            auto* userDataA = GetShapeUserData(beginEvent->shapeIdA);
            auto* userDataB = GetShapeUserData(beginEvent->shapeIdB);

            if (!userDataA || !userDataB) continue;

            if (!registry.valid(userDataA->ID) || !registry.valid(userDataB->ID))
            {
                KTN_CORE_ERROR("One of the entities involved in the contact begin event is invalid. ShapeA ID: '{}', ShapeB ID: '{}'", (uint32_t)userDataA->ID, (uint32_t)userDataB->ID);
                continue;
            }

            auto aID = registry.get<IDComponent>(userDataA->ID).ID;
            auto bID = registry.get<IDComponent>(userDataB->ID).ID;

            m_ActiveContactEvents.insert(std::make_pair(aID, bID));

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

            if (!registry.valid(userDataA->ID) || !registry.valid(userDataB->ID))
            {
                KTN_CORE_ERROR("One of the entities involved in the contact end event is invalid. ShapeA ID: '{}', ShapeB ID: '{}'", (uint32_t)userDataA->ID, (uint32_t)userDataB->ID);
                continue;
            }

            auto aID = registry.get<IDComponent>(userDataA->ID).ID;
            auto bID = registry.get<IDComponent>(userDataB->ID).ID;

            m_ActiveContactEvents.erase(std::make_pair(aID, bID));

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

        for (const auto& collisionPair : m_ActiveContactEvents)
        {
            auto aID = collisionPair.first;
            auto bID = collisionPair.second;

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

    void B2Physics::OverlapContactEvents(Scene* p_Scene)
    {
        KTN_PROFILE_FUNCTION();

        for (const auto& pair : m_FrameContactEvents)
        {
            if (m_ActiveFrameContactEvents.empty() || !m_ActiveFrameContactEvents.contains(pair))
            {
                m_ActiveFrameContactEvents.insert(pair);

                UUID idA = pair.first;
                UUID idB = pair.second;

                auto aInstance = ScriptEngine::GetEntityScriptInstance(idA);
                if (aInstance)
                {
                    void* params[] = { &idB };
                    aInstance->TryInvokeMethod("OnCollisionEnter", 1, params);
                }

                auto bInstance = ScriptEngine::GetEntityScriptInstance(idB);
                if (bInstance)
                {
                    void* params[] = { &idA };
                    bInstance->TryInvokeMethod("OnCollisionEnter", 1, params);
                }
            }
        }

        for (const auto& pair : m_FrameContactEvents)
        {
            if (!m_ActiveFrameContactEvents.empty() && m_ActiveFrameContactEvents.contains(pair))
            {
                UUID idA = pair.first;
                UUID idB = pair.second;

                auto aInstance = ScriptEngine::GetEntityScriptInstance(idA);
                if (aInstance)
                {
                    void* params[] = { &idB };
                    aInstance->TryInvokeMethod("OnCollisionStay", 1, params);
                }

                auto bInstance = ScriptEngine::GetEntityScriptInstance(idB);
                if (bInstance)
                {
                    void* params[] = { &idA };
                    bInstance->TryInvokeMethod("OnCollisionStay", 1, params);
                }
            }
        }

        std::unordered_set<std::pair<UUID, UUID>> toDelete;

        for (const auto& pair : m_ActiveFrameContactEvents)
        {
            if (m_FrameContactEvents.empty() || !m_FrameContactEvents.contains(pair))
            {
                toDelete.insert(pair);

                UUID idA = pair.first;
                UUID idB = pair.second;

                auto aInstance = ScriptEngine::GetEntityScriptInstance(idA);
                if (aInstance)
                {
                    void* params[] = { &idB };
                    aInstance->TryInvokeMethod("OnCollisionExit", 1, params);
                }

                auto bInstance = ScriptEngine::GetEntityScriptInstance(idB);
                if (bInstance)
                {
                    void* params[] = { &idA };
                    bInstance->TryInvokeMethod("OnCollisionExit", 1, params);
                }
            }
        }

        for (const auto& pair : toDelete)
        {
            m_ActiveFrameContactEvents.erase(pair);
        }
    }

    void B2Physics::CreateShape(Scene* p_Scene, Entity p_Entt, PhysicsBody2D* p_PhyBody, BodyShape2DComponent& p_Shape, const glm::vec3& p_Scale)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Scene || !p_Entt)
        {
            KTN_CORE_ERROR("Scene or Entity is null!");
            return;
        }

        if (!p_PhyBody)
        {
            KTN_CORE_ERROR("PhysicsBody2D is null!");
            return;
        }

        auto* userData = new ShapeUserData{
            .ID = p_Entt.GetHandle(),
            .IsTrigger = p_PhyBody->IsTrigger
        };
        m_ToDelete.push_back(userData);

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        auto material = AssetManager::Get()->GetAsset<PhysicsMaterial2D>(p_PhyBody->PhysicsMaterial2D);
        shapeDef.density = p_PhyBody->Mass;
        shapeDef.material.friction = material->Friction;
        shapeDef.material.restitution = material->Restitution;
        shapeDef.material.rollingResistance = material->RestitutionThreshold;
        shapeDef.userData = userData;
        shapeDef.enableSensorEvents = true;
        shapeDef.enableContactEvents = !p_PhyBody->IsTrigger;
        shapeDef.filter.categoryBits = GetFilterCategory(p_PhyBody);
        shapeDef.filter.maskBits = CategoryTrigger | CategoryStatic | CategoryCharacter | CategoryDynamic;

        if (p_PhyBody->IsTrigger)
        {
            shapeDef.isSensor = true;
            shapeDef.density = 0.0f;
        }

        b2BodyId body = GetBodyID(p_PhyBody->Body);

        if (p_Shape.Shape == Shape2D::Rect)
        {
            b2Polygon box = b2MakeOffsetBox(p_Shape.Size.x * p_Scale.x, p_Shape.Size.y * p_Scale.y, b2Vec2(p_Shape.Offset.x, p_Shape.Offset.y), b2MakeRot(0.0f));
            b2CreatePolygonShape(body, &shapeDef, &box);
        }

        if (p_Shape.Shape == Shape2D::Circle)
        {
            b2Circle circle = { { p_Shape.Offset.x, p_Shape.Offset.y }, p_Shape.Size.x * p_Scale.x };
            b2CreateCircleShape(body, &shapeDef, &circle);
        }
    }

    void B2Physics::CreatePhysicsBody(Scene* p_Scene, Entity p_Entt, TransformComponent& p_Transform, PhysicsBody2D* p_Body)
    {
        KTN_PROFILE_FUNCTION();

        if (!p_Scene || !p_Entt)
        {
            KTN_CORE_ERROR("Scene or Entity is null!");
            return;
        }

        if (!p_Body)
        {
            KTN_CORE_ERROR("PhysicsBody2D is null!");
            return;
        }

        b2BodyDef bodyDef = b2DefaultBodyDef();

        bodyDef.gravityScale = 0.0f;
        bodyDef.fixedRotation = true;
        bodyDef.linearDamping = 0.0f;
        bodyDef.angularDamping = 0.0f;
        bodyDef.isEnabled = p_Entt.IsEnabled();

        switch (p_Body->GetType())
        {
            case Rigidbody2DComponent::Type::Static:
                bodyDef.type = b2_staticBody;
                break;
            case Rigidbody2DComponent::Type::Rigidbody:
            {
                bodyDef.type = b2_dynamicBody;
                auto* rc = static_cast<Rigidbody2DComponent*>(p_Body);
                bodyDef.gravityScale = rc->GravityScale;
                bodyDef.fixedRotation = rc->FixedRotation;
                bodyDef.linearDamping = rc->LinearDamping;
                bodyDef.angularDamping = rc->AngularDamping;
                bodyDef.isAwake = !rc->Sleeping;
                bodyDef.enableSleep = rc->CanSleep;
                break;
            }
            case Rigidbody2DComponent::Type::Character:
                bodyDef.type = b2_kinematicBody;
                break;
        }

        glm::vec3 pos = p_Transform.GetWorldTranslation();
        glm::vec3 scale = p_Transform.GetWorldScale();
        glm::vec3 rot = p_Transform.GetWorldRotation();

        bodyDef.position = { pos.x, pos.y };
        bodyDef.rotation = b2MakeRot(rot.z);

        b2WorldId world = GetWorldID(m_World);
        b2BodyId body = b2CreateBody(world, &bodyDef);

        p_Body->Body.Index = body.index1;
        p_Body->Body.World = body.world0;
        p_Body->Body.Generation = body.generation;

        if (p_Entt.HasComponent<BodyShape2DComponent>())
        {
            auto& shape = p_Entt.GetComponent<BodyShape2DComponent>();
            CreateShape(p_Scene, p_Entt, p_Body, shape, scale);
        }
    }

} // namespace KTN
