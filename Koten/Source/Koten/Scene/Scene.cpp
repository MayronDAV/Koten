#include "ktnpch.h"
#include "Scene.h"
#include "Koten/Graphics/Renderer.h"
#include "Entity.h"
#include "Koten/Systems/B2Physics.h"
#include "Koten/Graphics/DebugRenderer.h"
#include "Koten/Script/ScriptEngine.h"
#include "Koten/Graphics/DFFont.h"
#include "Koten/Project/Project.h"
#include "Koten/Graphics/Material.h"
#include "Koten/Systems/AnimSystem.h"
#include "Koten/Graphics/PickingManager.h"
#include "Koten/Graphics/UISystem.h"




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

        static Entity DuplicateEntityRecursive(Scene* p_Scene, entt::registry& p_Registry, entt::entity p_Source, entt::entity p_NewParent)
        {
            KTN_PROFILE_FUNCTION_LOW();

            const auto& tag  = p_Registry.get<TagComponent>(p_Source).Tag;
            Entity newEntity = p_Scene->CreateEntity(tag);

            CopyEntity<ALL_COMPONENTS>(p_Source, (entt::entity)newEntity, p_Registry, p_Registry);

            if (p_Scene->GetSystemManager()->HasSystem<B2Physics>())
            {
                auto system = p_Scene->GetSystemManager()->GetSystem<B2Physics>();
                if (system->IsRunning())
                {
                    system->OnCreateEntity(newEntity);
                }
            }

            auto& newHierarchy    = p_Registry.get_or_emplace<HierarchyComponent>((entt::entity)newEntity);
            newHierarchy.Parent   = p_NewParent;
            newHierarchy.First    = entt::null;
            newHierarchy.Next     = entt::null;
            newHierarchy.Prev     = entt::null;

            auto* sourceHierarchy = p_Registry.try_get<HierarchyComponent>(p_Source);
            if (!sourceHierarchy)
                return newEntity;

            entt::entity prevChildNew = entt::null;
            entt::entity child        = sourceHierarchy->First;

            while (child != entt::null && p_Registry.valid(child))
            {
                Entity newChild            = DuplicateEntityRecursive(p_Scene, p_Registry, child, (entt::entity)newEntity);

                auto& childHierarchyNew    = p_Registry.get<HierarchyComponent>((entt::entity)newChild);

                if (prevChildNew == entt::null)
                {
                    newHierarchy.First     = (entt::entity)newChild;
                    childHierarchyNew.Prev = entt::null;
                }
                else
                {
                    auto& prevHierarchyNew = p_Registry.get<HierarchyComponent>(prevChildNew);

                    prevHierarchyNew.Next  = (entt::entity)newChild;
                    childHierarchyNew.Prev = prevChildNew;
                }

                childHierarchyNew.Next  = entt::null;
                prevChildNew            = (entt::entity)newChild;
                auto* childHierarchyOld = p_Registry.try_get<HierarchyComponent>(child);
                child                   = childHierarchyOld ? childHierarchyOld->Next : entt::null;
            }

            return newEntity;
        }

    } // namespace

    Scene::Scene()
    {
        KTN_PROFILE_FUNCTION();

        m_Registry.ctx().emplace<Scene*>(this);

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
        AddDependency<UIInputComponent, UIComponent>(m_Registry);
        AddDependency<UIImageComponent, UIComponent>(m_Registry);

        RegisterComponentCallbacks<ALL_COMPONENTS>(m_Registry);

        m_SystemManager = CreateUnique<SystemManager>();
        if (m_Config.UsePhysics2D)
            m_SystemManager->RegisterSystem<B2Physics>();
        m_SystemManager->RegisterSystem<AnimSystem>();

        m_SceneGraph = CreateUnique<SceneGraph>();
        m_SceneGraph->Init(m_Registry);

        m_UISystem = CreateRef<UISystem>();
        m_UISystem->Init(this);
    }

    Scene::~Scene()
    {
    }

    void Scene::Copy(const Ref<Scene>& p_Src, const Ref<Scene>& p_Dest)
    {
        KTN_PROFILE_FUNCTION();

        p_Dest->Handle         = p_Src->Handle;
        p_Dest->m_Width        = p_Src->m_Width;
        p_Dest->m_Height       = p_Src->m_Height;
        p_Dest->m_RenderTarget = p_Src->m_RenderTarget;
        p_Dest->m_Projection   = p_Src->m_Projection;
        p_Dest->m_View         = p_Src->m_View;
        p_Dest->m_ClearColor   = p_Src->m_ClearColor;
        p_Dest->m_HaveCamera   = p_Src->m_HaveCamera;

        auto& srcRegistry      = p_Src->m_Registry;
        auto& destRegistry     = p_Dest->m_Registry;
        std::unordered_map<UUID, std::pair<entt::entity, entt::entity>> enttMap;

        p_Dest->m_SceneGraph->DisableOnConstruct(destRegistry, true);

        srcRegistry.view<IDComponent, TagComponent>().each(
        [&](auto p_Entity, IDComponent& p_ID, TagComponent& p_Tag)
        {
            UUID uuid         = p_ID.ID;
            const auto& name  = p_Tag.Tag;

            Entity newEntity  = p_Dest->CreateEntity(uuid, name);
            enttMap[uuid]     = std::make_pair(p_Entity, newEntity.GetHandle());

            CopyEntity<ALL_COMPONENTS>(p_Entity, (entt::entity)newEntity, srcRegistry, destRegistry);

            auto hcomp        = newEntity.TryGetComponent<HierarchyComponent>();
            if (hcomp)
            {
                hcomp->Parent = entt::null;
                hcomp->First  = entt::null;
                hcomp->Prev   = entt::null;
                hcomp->Next   = entt::null;
            }
        });

        destRegistry.view<IDComponent, HierarchyComponent>().each(
        [&](auto p_Entity, IDComponent& p_ID, HierarchyComponent& p_HC)
        {
            auto srcEntt    = enttMap[p_ID.ID].first;

            auto& srcHC     = srcRegistry.get<HierarchyComponent>(srcEntt);
            p_HC.Parent     = srcHC.Parent != entt::null ? enttMap[srcRegistry.get<IDComponent>(srcHC.Parent).ID].second : entt::null;
            p_HC.First      = srcHC.First  != entt::null ? enttMap[srcRegistry.get<IDComponent>(srcHC.First).ID].second  : entt::null;
            p_HC.Prev       = srcHC.Prev   != entt::null ? enttMap[srcRegistry.get<IDComponent>(srcHC.Prev).ID].second   : entt::null;
            p_HC.Next       = srcHC.Next   != entt::null ? enttMap[srcRegistry.get<IDComponent>(srcHC.Next).ID].second   : entt::null;
            p_HC.ChildCount = srcHC.ChildCount;
        });

        p_Dest->m_SceneGraph->DisableOnConstruct(destRegistry, false);
    }

    Entity Scene::DuplicateEntity(const Entity& p_Entity)
    {
        KTN_PROFILE_FUNCTION();

        Scene* scene = p_Entity.GetScene();
        auto& registry = scene->GetRegistry();

        return DuplicateEntityRecursive(scene, registry, p_Entity, entt::null);
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

    void Scene::UpdateRenderList()
    {
        KTN_PROFILE_FUNCTION();

        m_RenderList.Clear();

        DebugRenderer::Begin(&m_RenderList);
        m_Registry.view<RuntimeComponent, TransformComponent>().each(
        [&](auto p_Entity, const RuntimeComponent& p_Runtime, const TransformComponent& p_Transform)
        {
            if (!p_Runtime.Active) return;

            auto entt = Entity(p_Entity, this);
            if (entt.HasComponent<UIComponent>() || entt.HasComponent<UICanvasComponent>())
                return;

            auto& settings = Engine::Get().GetSettings();
            auto shape2d   = entt.TryGetComponent<BodyShape2DComponent>();
            if (shape2d && settings.ShowDebugPhysicsCollider)
                DebugRenderer::DrawCollider2D(entt, { 1.0f, 0.65f, 0.0f, 1.0f });

            RenderCommand command = {};
            command.ID            = PickingManager::RegisterEntity(entt);
            command.Transform     = p_Transform.GetWorldMatrix();

            auto sprite = entt.TryGetComponent<SpriteComponent>();
            if (sprite)
            {
                SpriteCommand spriteCommand    = {};
                spriteCommand.Type             = sprite->Type;
                spriteCommand.Thickness        = sprite->Thickness;
                spriteCommand.Fade             = sprite->Fade;

                auto mat                       = AssetManager::Get()->GetAsset<Material>(sprite->Material);
                spriteCommand.Color            = mat->AlbedoColor;

                auto animComp                  = entt.TryGetComponent<AnimationComponent>();
                if (animComp)
                {
                    spriteCommand.Texture      = AssetManager::Get()->GetAsset<Texture2D>(animComp->Texture);
                    spriteCommand.UseDirectUVs = true;
                    spriteCommand.UV           = animComp->CurrentAnim.UV;
                }
                else
                {
                    spriteCommand.Texture      = AssetManager::Get()->GetAsset<Texture2D>(mat->Texture);
                    spriteCommand.Size         = sprite->Size;
                    spriteCommand.BySize       = sprite->BySize;
                    spriteCommand.Offset       = sprite->Offset;
                    spriteCommand.Scale        = sprite->Scale;
                }

                command.Command                = spriteCommand;
                m_RenderList.Submit(command);
            }

            auto line                   = entt.TryGetComponent<LineRendererComponent>();
            if (line)
            {
                LineCommand lineCommand = {};
                lineCommand.Primitive   = line->Primitive;
                lineCommand.Color       = line->Color;
                lineCommand.Width       = line->Width;
                lineCommand.Start       = line->Start;
                lineCommand.End         = line->End;

                command.Command         = lineCommand;

                m_RenderList.Submit(command);
            }

            auto text                   = entt.TryGetComponent<TextRendererComponent>();
            if (text)
            {
                TextCommand textCommand = {};
                textCommand.Font        = AssetManager::Get()->GetAsset<DFFont>(text->Font);
                textCommand.Text        = text->String;
                textCommand.Color       = text->Color;
                textCommand.BgColor     = text->BgColor;
                textCommand.CharBgColor = text->CharBgColor;
                textCommand.DrawBg      = text->DrawBg;
                textCommand.LineSpacing = text->LineSpacing;
                textCommand.Kerning     = text->Kerning;

                command.Command         = textCommand;

                m_RenderList.Submit(command);
            }
        });
        DebugRenderer::End();
    }

    void Scene::OnUpdate()
    {
        KTN_PROFILE_FUNCTION();

        RemoveSystems();

        m_SceneGraph->Update(m_Registry);

        m_UISystem->Reset();

        bool first = true;
        m_Registry.view<TransformComponent, CameraComponent>().each(
        [&](auto p_Entt, TransformComponent& p_Transform, CameraComponent& p_Camera)
        {
            if (!p_Camera.RenderTarget)
                p_Camera.Camera.SetViewportSize(m_Width, m_Height);

            p_Camera.Camera.OnUpdate();

            if (!p_Camera.RenderTarget)
            {
                if (!first)
                {
                    KTN_CORE_ERROR("there can only be one primary camera!");
                    return;
                }

                m_Projection = p_Camera.Camera.GetProjection();
                m_View       = glm::inverse(p_Transform.GetWorldMatrix());
                m_ClearColor = p_Camera.ClearColor;
                m_HaveCamera = true;
                first        = false;
            }

            if (p_Camera.RenderTarget)
            {
                auto renderTarget = AssetManager::Get()->GetAsset<Texture2D>(p_Camera.RenderTarget);
                if (renderTarget)
                {
                    renderTarget->Resize(p_Camera.Camera.GetViewportWidth(), p_Camera.Camera.GetViewportHeight());
                }
            }
        });

        UpdateRenderList();
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

        m_UISystem->Reset();

        bool first = true;
        m_Registry.view<TransformComponent, CameraComponent>().each(
        [&](auto p_Entt, TransformComponent& p_Transform, CameraComponent& p_Camera)
        {
            if (!p_Camera.RenderTarget)
                p_Camera.Camera.SetViewportSize(m_Width, m_Height);

            p_Camera.Camera.OnUpdate();

            if (!p_Camera.RenderTarget)
            {
                if (!first)
                {
                    KTN_CORE_ERROR("there can only be one primary camera!");
                    return;
                }

                m_Projection = p_Camera.Camera.GetProjection();
                m_View       = glm::inverse(p_Transform.GetWorldMatrix());
                m_ClearColor = p_Camera.ClearColor;
                m_HaveCamera = true;
                first        = false;
            }

            if (p_Camera.RenderTarget)
            {
                auto renderTarget = AssetManager::Get()->GetAsset<Texture2D>(p_Camera.RenderTarget);
                if (renderTarget)
                {
                    renderTarget->Resize(p_Camera.Camera.GetViewportWidth(), p_Camera.Camera.GetViewportHeight());
                }
            }
        });

        UpdateRenderList();
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

        m_UISystem->Reset();

        bool first = true;
        m_Registry.view<TransformComponent, CameraComponent>().each(
        [&](auto p_Entt, TransformComponent& p_Transform, CameraComponent& p_Camera)
        {
            if (!p_Camera.RenderTarget)
                p_Camera.Camera.SetViewportSize(m_Width, m_Height);

            p_Camera.Camera.OnUpdate();

            if (!p_Camera.RenderTarget)
            {
                if (!first)
                {
                    KTN_CORE_ERROR("there can only be one primary camera!");
                    return;
                }

                m_Projection = p_Camera.Camera.GetProjection();
                m_View       = glm::inverse(p_Transform.GetWorldMatrix());
                m_ClearColor = p_Camera.ClearColor;
                m_HaveCamera = true;
                first        = false;
            }

            if (p_Camera.RenderTarget)
            {
                auto renderTarget = AssetManager::Get()->GetAsset<Texture2D>(p_Camera.RenderTarget);
                if (renderTarget)
                {
                    renderTarget->Resize(p_Camera.Camera.GetViewportWidth(), p_Camera.Camera.GetViewportHeight());
                }
            }
        });

        UpdateRenderList();
    }

    void Scene::OnRender(const glm::mat4& p_Projection, const glm::mat4& p_View, const glm::vec4& p_ClearColor)
    {
        KTN_PROFILE_FUNCTION();

        RenderScene(p_Projection, p_View, p_ClearColor);
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

    void Scene::SetViewportSize(uint32_t p_Width, uint32_t p_Height, const glm::vec2& p_LeftTop)
    {
        KTN_PROFILE_FUNCTION();

        m_Width  = p_Width;
        m_Height = p_Height;

        m_UISystem->SetLeftTop(p_LeftTop);
    }

    void Scene::SetEntityTransform(Entity p_Entity, const glm::vec3& p_Pos, const glm::vec3& p_Rot)
    {
        KTN_PROFILE_FUNCTION();

        if ((p_Entity.HasComponent<CharacterBody2DComponent>() || p_Entity.HasComponent<Rigidbody2DComponent>() || p_Entity.HasComponent<StaticBody2DComponent>()) &&
            m_SystemManager->HasSystem<B2Physics>())
        {
            m_SystemManager->GetSystem<B2Physics>()->SetTransform(p_Entity, p_Pos, p_Rot.z);
        }

        auto tcomp = p_Entity.TryGetComponent<TransformComponent>();
        if (tcomp)
        {
            tcomp->SetLocalTranslation(p_Pos);
            tcomp->SetLocalRotation(p_Rot);
        }
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

    void Scene::RenderScene(const glm::mat4& p_Projection, const glm::mat4& p_View, const glm::vec4& p_ClearColor)
    {
        KTN_PROFILE_FUNCTION();

        m_Registry.view<TransformComponent, CameraComponent>().each(
        [&](auto p_Entt, TransformComponent& p_Transform, CameraComponent& p_Camera)
        {
            if (!p_Camera.RenderTarget)
                return;

            auto renderTarget   = AssetManager::Get()->GetAsset<Texture2D>(p_Camera.RenderTarget);
            if (!renderTarget)
                return;

            RenderPassInfo info = {};
            info.RenderTarget   = renderTarget;
            info.Width          = renderTarget->GetWidth();
            info.Height         = renderTarget->GetHeight();
            info.Projection     = p_Camera.Camera.GetProjection();
            info.View           = glm::inverse(p_Transform.GetWorldMatrix());
            info.Clear          = true;
            info.ClearColor     = p_Camera.ClearColor;

            Renderer::BeginPass(info);
            {
                Renderer::Submit(m_RenderList);
            }
            Renderer::EndPass();
        });

        if (m_HaveCamera)
        {
            RenderPassInfo info  = {};
            info.RenderTarget    = m_RenderTarget;
            info.PickingTarget   = m_PickingTarget;
            info.Width           = m_Width;
            info.Height          = m_Height;
            info.Projection      = p_Projection;
            info.View            = p_View;
            info.Clear           = true;
            info.ClearColor      = p_ClearColor;
            info.Picking         = Engine::Get().GetSettings().MousePicking && m_PickingTarget;

            Renderer::BeginPass(info);
            {
                Renderer::Submit(m_RenderList);
            }
            Renderer::EndPass();
        }

        m_UISystem->Update();
        m_UISystem->ProcessCanvas();
        m_UISystem->Render();
    }

    void Scene::OnRenderRuntime()
    {
        KTN_PROFILE_FUNCTION();

        RenderScene(m_Projection, m_View, m_ClearColor);
    }

} // namespace KTN
