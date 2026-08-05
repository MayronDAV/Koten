#include "ktnpch.h"
#include "Renderer.h"
#include "Koten/Graphics/Pipeline.h"
#include "Koten/Graphics/Shader.h"
#include "Koten/Graphics/DescriptorSet.h"
#include "Koten/Graphics/RendererCommand.h"
#include "Koten/Core/Application.h"
#include "Koten/Core/TaskManager.h"

// std
#include <codecvt>
#include <locale>
#include <numeric>
#include <mutex>
#include <typeindex>



namespace KTN
{
    namespace
    {
        struct DrawElementsIndirectCommand
        {
            uint32_t Count;
            uint32_t InstanceCount;
            uint32_t FirstIndex;
            uint32_t BaseVertex;
            uint32_t BaseInstance;
        };

        struct DrawIndirectCommand 
        {
            uint32_t VertexCount;
            uint32_t InstanceCount;
            uint32_t FirstVertex;
            uint32_t BaseInstance;
        };

        struct EntityBufferData
        {
            int Count = 0;
            std::vector<int> EntityIDS;
        };

        struct RendererResources
        {
            Ref<Texture2D> WhiteTexture        = nullptr;

            Ref<Shader> FinalShader            = nullptr;
            Ref<DescriptorSet> FinalSet        = nullptr;

            Ref<Shader> FinalPickingShader     = nullptr;
            Ref<DescriptorSet> FinalPickingSet = nullptr;
        };

        struct RenderTargets
        {
            Ref<Texture2D> Color;
            Ref<Texture2D> Depth;
            Ref<Texture2D> Resolve;
            Ref<Texture2D> Picking;
            Ref<Texture2D> PickingDepth;

            bool ClearedThisFrame = false;
        };

        // maybe move to a header file if needed elsewhere
        template <typename TBatch>
        class BatchList
        {
            public:
                TBatch& NewBatch()
                {
                    KTN_PROFILE_FUNCTION();

                    if (m_Count >= m_Batches.size())
                        m_Batches.emplace_back();

                    return m_Batches[m_Count++];
                }

                template<typename... Args>
                TBatch& NewBatch(Args&&... p_Args)
                {
                    KTN_PROFILE_FUNCTION();

                    if (m_Count >= m_Batches.size())
                        m_Batches.emplace_back(std::forward<Args>(p_Args)...);

                    return m_Batches[m_Count++];
                }

                void Clear()
                {
                    m_Count = 0;
                }

                bool Empty() const { return m_Count == 0 || m_Batches.empty(); }
                void Reserve(size_t p_Size) { m_Batches.reserve(p_Size); }

                TBatch& GetOrCreateBatch()
                {
                    if (Empty())
                        return NewBatch();

                    return back();
                }

                TBatch& operator[](size_t p_Index) { return m_Batches[p_Index]; }
                const TBatch& operator[](size_t p_Index) const { return m_Batches[p_Index]; }

                TBatch& back() { return m_Batches.back(); }

                std::vector<TBatch>::iterator begin() { return m_Batches.begin(); }
                std::vector<TBatch>::iterator end() { return m_Batches.end(); }
                std::vector<TBatch>::reverse_iterator rbegin() { return m_Batches.rbegin(); }
                std::vector<TBatch>::reverse_iterator rend() { return m_Batches.rend(); }

                std::vector<TBatch>::const_iterator begin()          const { return m_Batches.begin(); }
                std::vector<TBatch>::const_iterator end()            const { return m_Batches.end(); }
                std::vector<TBatch>::const_reverse_iterator rbegin() const { return m_Batches.rbegin(); }
                std::vector<TBatch>::const_reverse_iterator rend()   const { return m_Batches.rend(); }

            private:
                std::vector<TBatch> m_Batches;
                uint32_t m_Count = 0;
        };

        struct IRenderPassData
        {
            virtual ~IRenderPassData() = default;
        };

        struct RenderPass
        {
            RenderPass() = default;
            ~RenderPass() = default;

            RenderPass(const RenderPass&) = delete;
            RenderPass& operator=(const RenderPass&) = delete;

            RenderPass(RenderPass&&) noexcept = default;
            RenderPass& operator=(RenderPass&&) noexcept = default;

            Ref<RenderTargets> Targets = nullptr;
            RenderPassInfo Info;
            std::vector<RenderList> Lists;

            template<typename T, typename... Args>
            T& GetData(Args&&... p_Args)
            {
                auto key     = std::type_index(typeid(T));

                auto it      = m_Data.find(key);
                if (it == m_Data.end())
                {
                    auto ptr = std::make_unique<T>(std::forward<Args>(p_Args)...);
                    T* raw   = ptr.get();
                    m_Data.emplace(key, std::move(ptr));

                    return *raw;
                }

                return *static_cast<T*>(it->second.get());
            }

            private:
                std::unordered_map<std::type_index, std::unique_ptr<IRenderPassData>> m_Data;
        };

        struct FrameData
        {
            std::vector<RenderPass> Passes;
            RenderPass* CurrentPass = nullptr;
        };

        struct RendererState
        {
            RendererResources Resources;
            std::unordered_map<uint64_t, Ref<RenderTargets>> TargetsCache;
            FrameData Frame;
        };

        class SpriteRenderer
        {
            public:
                SpriteRenderer() { Init(); }
                ~SpriteRenderer() = default;
                void Init();
                void Begin();
                void Render(RenderPass& p_Pass);


            private:
                struct Resources
                {
                    Ref<Shader> MainShader;
                    Ref<DescriptorSet> MainSet;
                    Ref<Shader> PickingShader;
                    Ref<DescriptorSet> PickingSet;
                    Ref<VertexArray> VAO;
                    Ref<IndirectBuffer> IndirectBuffer;
                };

                struct InstanceData
                {
                    glm::mat4 Transform;
                    glm::vec4 Color;
                    glm::vec4 UV;
                    glm::vec4 Others; // Type, TexIndex, Thickness, Fade
                };

                struct Batch
                {
                    std::vector<InstanceData> Instances;
                    std::vector<PickingID> PickingIDs;
                    uint32_t TextureSlot = 1;
                    std::array<Ref<Texture2D>, MAX_TEXTURE_SLOTS> Textures;

                    void Reset(const Ref<Texture2D>& p_WhiteTexture)
                    {
                        Instances.clear();
                        PickingIDs.clear();
                        TextureSlot = 1;
                        Textures.fill(nullptr);
                        Textures[0] = p_WhiteTexture;
                    }
                };

                struct SpritePassData : IRenderPassData
                {
                    BatchList<Batch> Batches;
                };

            private:
                void Build(RenderPass& p_Pass);
                void Draw(RenderPass& p_Pass);

            private:
                Resources m_Resources;
        };

        class LineRenderer
        {
            public:
                LineRenderer() { Init(); }
                ~LineRenderer() = default;
                void Init();
                void Begin();
                void Render(RenderPass& p_Pass);

            private:
                struct Resources
                {
                    Ref<Shader> PrimitiveShader;
                    Ref<DescriptorSet> PrimitiveSet;
                    Ref<Shader> NonPrimitiveShader;
                    Ref<DescriptorSet> NonPrimitiveSet;
                    Ref<IndirectBuffer> IndirectBuffer;
                };

                struct InstanceData
                {
                    glm::mat4 Transform;
                    glm::vec4 Start;
                    glm::vec4 End;
                    glm::vec4 Color;
                    alignas(16) float Width;
                };

                struct Batch
                {
                    std::vector<InstanceData> Instances;
                };

                struct LinePassData : IRenderPassData
                {
                    std::unordered_map<float, BatchList<Batch>> PrimitiveBatches;
                    BatchList<Batch> NonPrimitiveBatches;
                };

            private:
                void Build(RenderPass& p_Pass);
                void Draw(RenderPass& p_Pass);

            private:
                Resources m_Resources;
        };

        class TextRenderer
        {
            public:
                TextRenderer() { Init(); }
                ~TextRenderer() = default;
                void Init();
                void Begin();
                void Render(RenderPass& p_Pass);

            private:
                struct Resources
                {
                    Ref<Shader> MainShader;
                    Ref<DescriptorSet> MainSet;
                    Ref<Shader> PickingShader;
                    Ref<DescriptorSet> PickingSet;
                    Ref<IndirectBuffer> IndirectBuffer;
                };

                struct InstanceData
                {
                    glm::mat4 Transform;
                    glm::vec4 Positions;
                    glm::vec4 Color;
                    glm::vec4 BgColor;
                    glm::vec4 UV;
                    alignas(16) float TexIndex;
                };

                struct Batch
                {
                    std::vector<InstanceData> Instances;
                    std::vector<PickingID> PickingIDs;
                    uint32_t TextureSlot = 1;
                    std::array<Ref<Texture2D>, MAX_TEXTURE_SLOTS> FontAtlasTextures;

                    void Reset(const Ref<Texture2D>& p_WhiteTexture)
                    {
                        Instances.clear();
                        PickingIDs.clear();
                        TextureSlot = 1;
                        FontAtlasTextures.fill(p_WhiteTexture);
                    }
                };

                struct TextPassData : IRenderPassData
                {
                    BatchList<Batch> Batches;
                };

            private:
                void Build(RenderPass& p_Pass);
                void Draw(RenderPass& p_Pass);

            private:
                Resources m_Resources;
        };

        static std::u32string UTF8ToUTF32(const std::string_view& p_UTF8)
        {
            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
            return converter.from_bytes(p_UTF8.data());
        }

        static void Clear(RenderPass& p_Pass)
        {
            KTN_PROFILE_FUNCTION();

            if (p_Pass.Targets->Color)
                RendererCommand::ClearRenderTarget(p_Pass.Targets->Color, p_Pass.Info.ClearColor);
            if (p_Pass.Targets->Picking)
                RendererCommand::ClearRenderTarget(p_Pass.Targets->Picking, INVALID_PICKING_ID);
            if (p_Pass.Targets->Resolve)
                RendererCommand::ClearRenderTarget(p_Pass.Targets->Resolve, p_Pass.Info.ClearColor);
            if (p_Pass.Targets->Depth)
                RendererCommand::ClearRenderTarget(p_Pass.Targets->Depth, -1);
            if (p_Pass.Targets->PickingDepth)
                RendererCommand::ClearRenderTarget(p_Pass.Targets->PickingDepth, -1);
        }

    } // namespace

    static RendererState* s_Renderer = nullptr;
    static SpriteRenderer* s_Sprite  = nullptr;
    static LineRenderer* s_Line      = nullptr;
    static TextRenderer* s_Text      = nullptr;

    void Renderer::Init()
    {
        KTN_PROFILE_FUNCTION();

        s_Renderer                         = new RendererState();

        uint32_t whiteTextureData          = 0xffffffff;
        s_Renderer->Resources.WhiteTexture = Texture2D::Create({}, (uint8_t*)&whiteTextureData, sizeof(uint32_t));

        TaskManager::Get().AddTask({
            "FinalPassShader",
            TaskManager::Phase::Init,
            0,
            []()
            {
                auto spirvSource                      = Shader::CompileOrGetSpirv("Assets/Shaders/FinalPass.glsl");
                KTN_CORE_INFO("Compiled FinalPass shader!");
                Application::Get().SubmitToMainThread([source = std::move(spirvSource)]()
                {
                    s_Renderer->Resources.FinalShader = Shader::Create(source);
                    s_Renderer->Resources.FinalSet    = DescriptorSet::Create({ 0, s_Renderer->Resources.FinalShader });
                });
            },
            true,
            TaskManager::SyncPoint::None
        });

        TaskManager::Get().AddTask({
            "FinalPickingShader",
            TaskManager::Phase::Init,
            0,
            []()
            {
                auto spirvSource                             = Shader::CompileOrGetSpirv("Assets/Shaders/FinalPickingPass.glsl");
                KTN_CORE_INFO("Compiled FinalPickingPass shader!");
                Application::Get().SubmitToMainThread([source = std::move(spirvSource)]()
                {
                    s_Renderer->Resources.FinalPickingShader = Shader::Create(source);
                    s_Renderer->Resources.FinalPickingSet    = DescriptorSet::Create({ 0, s_Renderer->Resources.FinalPickingShader });
                });
            },
            true,
            TaskManager::SyncPoint::None
        });

        s_Sprite = new SpriteRenderer();
        s_Line   = new LineRenderer();
        s_Text   = new TextRenderer();
    }

    void Renderer::Shutdown()
    {
        KTN_PROFILE_FUNCTION();

        delete s_Renderer;
        delete s_Sprite;
        delete s_Line;
        delete s_Text;
    }

    void Renderer::BeginFrame()
    {
        KTN_PROFILE_FUNCTION();

        RendererCommand::Begin();

        s_Renderer->Frame.CurrentPass = nullptr;
        s_Renderer->Frame.Passes.clear();

        if (s_Sprite) s_Sprite->Begin();
        if (s_Line) s_Line->Begin();
        if (s_Text) s_Text->Begin();

        for (auto& [key, targets] : s_Renderer->TargetsCache)
            targets->ClearedThisFrame = false;
    }

    void Renderer::EndFrame()
    {
        KTN_PROFILE_FUNCTION();

        TaskManager::Get().WaitForSyncPoint(TaskManager::SyncPoint::FrameRender);

        std::unordered_map<uint64_t, std::pair<RenderPassInfo, Ref<RenderTargets>>> mapTargets;
        std::unordered_map<uint64_t, std::pair<RenderPassInfo, Ref<RenderTargets>>> pickingTargets;

        for (auto& pass : s_Renderer->Frame.Passes)
        {
            if (pass.Info.Clear && !pass.Targets->ClearedThisFrame)
            {
                Clear(pass);
                pass.Targets->ClearedThisFrame = true;
            }

            if (s_Sprite) s_Sprite->Render(pass);
            if (s_Line) s_Line->Render(pass);
            if (s_Text) s_Text->Render(pass);

            mapTargets[pass.Info.RenderTarget ? pass.Info.RenderTarget->Handle : (AssetHandle)0] = { pass.Info, pass.Targets };

            if (pass.Info.PickingTarget)
                pickingTargets[pass.Info.PickingTarget->Handle] = { pass.Info, pass.Targets };
        }

        auto commandBuffer              = RendererCommand::GetCurrentCommandBuffer();

        for (auto& [key, targets] : mapTargets )
        {
            PipelineSpecification pspec = {};
            pspec.pShader               = s_Renderer->Resources.FinalShader;
            pspec.ClearTargets          = true;
            pspec.DepthTest             = false;
            pspec.DepthWrite            = false;
            pspec.ClearColor            = targets.first.ClearColor;
            pspec.ColorTargets[0]       = targets.first.RenderTarget;
            pspec.SwapchainTarget       = targets.first.RenderTarget == nullptr;
            pspec.DebugName             = "FinalPassPipeline - " + (targets.first.RenderTarget ? std::to_string(targets.first.RenderTarget->Handle) : "Swapchain Target");

            auto pipeline               = Pipeline::Get(pspec);

            pipeline->Begin(commandBuffer);

            commandBuffer->SetViewport(0.0f, 0.0f, targets.first.Width, targets.first.Height);

            s_Renderer->Resources.FinalSet->SetTexture("u_Texture", targets.second->Color);
            s_Renderer->Resources.FinalSet->Upload(commandBuffer);

            commandBuffer->BindSets(&s_Renderer->Resources.FinalSet);
            RendererCommand::Draw(DrawType::TRIANGLES, nullptr, 6);

            pipeline->End(commandBuffer);
        }

        for (auto& [key, targets] : pickingTargets)
        {
            PipelineSpecification pspec = {};
            pspec.pShader               = s_Renderer->Resources.FinalPickingShader;
            pspec.ClearTargets          = true;
            pspec.DepthTest             = false;
            pspec.DepthWrite            = false;
            pspec.ClearColor            = targets.first.ClearColor;
            pspec.ColorTargets[0]       = targets.first.PickingTarget;
            pspec.SwapchainTarget       = false;
            pspec.DebugName             = "FinalPassPipeline - Picking - " + std::to_string(targets.first.PickingTarget->Handle);

            auto pipeline               = Pipeline::Get(pspec);

            pipeline->Begin(commandBuffer);

            commandBuffer->SetViewport(0.0f, 0.0f, targets.first.Width, targets.first.Height);

            s_Renderer->Resources.FinalPickingSet->SetTexture("u_Texture", targets.second->Picking);
            s_Renderer->Resources.FinalPickingSet->Upload(commandBuffer);

            commandBuffer->BindSets(&s_Renderer->Resources.FinalPickingSet);
            RendererCommand::Draw(DrawType::TRIANGLES, nullptr, 6);

            pipeline->End(commandBuffer);
        }

        RendererCommand::End();
    }

    void Renderer::BeginPass(const RenderPassInfo& p_PassInfo)
    {
        KTN_PROFILE_FUNCTION();

        TaskManager::Get().ExecutePhase(TaskManager::Phase::RenderPass);

        auto hash                     = p_PassInfo.RenderTarget ? (uint64_t)p_PassInfo.RenderTarget->Handle : (uint64_t)0;
        HashCombine(hash, p_PassInfo.Width, p_PassInfo.Height, p_PassInfo.Samples);

        auto& pass                    = s_Renderer->Frame.Passes.emplace_back();
        pass.Info                     = p_PassInfo;
        s_Renderer->Frame.CurrentPass = &pass;

        auto it = s_Renderer->TargetsCache.find(hash);
        if (it != s_Renderer->TargetsCache.end())
        {
            pass.Targets              = it->second;
            return;
        }

        auto targets                  = CreateRef<RenderTargets>();
        s_Renderer->TargetsCache.emplace(hash, targets);

        std::string text              = std::to_string(hash);

        TextureSpecification tspec    = {};
        tspec.Usage                   = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
        tspec.Width                   = p_PassInfo.Width;
        tspec.Height                  = p_PassInfo.Height;
        tspec.GenerateMips            = false;
        tspec.AnisotropyEnable        = false;
        tspec.Samples                 = p_PassInfo.Samples;
        tspec.Format                  = TextureFormat::RGBA32_FLOAT;
        tspec.DebugName               = "Pass - ColorTarget " + text;

        targets->Color                = Texture2D::Get(tspec);

        tspec.Samples                 = 1;
        if (p_PassInfo.Picking)
        {
            tspec.Format              = TextureFormat::R32_UINT;
            tspec.DebugName           = "Pass - PickingTarget " + text;

            targets->Picking          = Texture2D::Get(tspec);
        }


        if (p_PassInfo.Samples > 1)
        {
            tspec.Format              = TextureFormat::RGBA32_FLOAT;
            tspec.DebugName           = "Pass - ResolveTarget " + text;

            targets->Resolve          = Texture2D::Get(tspec);
        }

        tspec.Format                  = TextureFormat::D32_FLOAT;
        tspec.Usage                   = TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT;
        tspec.DebugName               = "Pass - DepthTarget " + text;

        targets->Depth                = Texture2D::Get(tspec);

        if (p_PassInfo.Picking)
        {
            tspec.Format              = TextureFormat::D32_FLOAT;
            tspec.DebugName           = "Pass - PickingDepthTarget " + text;

            targets->PickingDepth     = Texture2D::Get(tspec);
        }

        pass.Targets                  = targets;
    }

    void Renderer::EndPass()
    {
        KTN_PROFILE_FUNCTION();

        s_Renderer->Frame.CurrentPass = nullptr;
    }

    void Renderer::Submit(const RenderList& p_RenderList)
    {
        KTN_PROFILE_FUNCTION();

        s_Renderer->Frame.CurrentPass->Lists.push_back(p_RenderList);
    }

    namespace
    {
        #pragma region SpriteRenderer

        void SpriteRenderer::Init()
        {
            KTN_PROFILE_FUNCTION();

            TaskManager::Get().AddTask({
                "SpriteRenderer::Init",
                TaskManager::Phase::Init,
                1,
                [this]()
                {
                    auto spirvSource           = Shader::CompileOrGetSpirv("Assets/Shaders/R2D_Shader.glsl");
                    KTN_CORE_INFO("Compiled R2D_Shader shader!");
                    Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                    {
                        m_Resources.MainShader = Shader::Create(source);
                        m_Resources.MainSet    = DescriptorSet::Create({ 0, m_Resources.MainShader });
                    });
                },
                true,
                TaskManager::SyncPoint::None
            });

            if (Engine::Get().GetSettings().MousePicking)
            {
                TaskManager::Get().AddTask({
                    "SpriteRenderer::Init MousePicking",
                    TaskManager::Phase::Init,
                    2,
                    [this]()
                    {
                        auto spirvSource              = Shader::CompileOrGetSpirv("Assets/Shaders/R2D_Picking.glsl");
                        KTN_CORE_INFO("Compiled R2D_Picking shader!");
                        Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                        {
                            m_Resources.PickingShader = Shader::Create(source);
                            m_Resources.PickingSet    = DescriptorSet::Create({ 0, m_Resources.PickingShader });
                        });
                    },
                    true,
                    TaskManager::SyncPoint::None
                });
            }

            m_Resources.VAO = VertexArray::Create();

            float vertices[] = {
                // positions
                -0.5f, -0.5f, 0.0f,
                 0.5f, -0.5f, 0.0f,
                 0.5f,  0.5f, 0.0f,
                -0.5f,  0.5f, 0.0f
            };

            auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
            vbo->SetLayout({
                { DataType::Float3 , "a_Position"    }
            });
            m_Resources.VAO->SetVertexBuffer(vbo);

            uint32_t indices[] = {
                0, 1, 3, // first triangle
                1, 2, 3  // second triangle
            };
            auto ebo = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
            m_Resources.VAO->SetIndexBuffer(ebo);

            m_Resources.IndirectBuffer = IndirectBuffer::Create(sizeof(DrawElementsIndirectCommand));
        }

        void SpriteRenderer::Begin()
        {
            KTN_PROFILE_FUNCTION();

        }

        void SpriteRenderer::Render(RenderPass& p_Pass)
        {
            KTN_PROFILE_FUNCTION();

            Build(p_Pass);

            Draw(p_Pass);
        }

        void SpriteRenderer::Build(RenderPass& p_Pass)
        {
            KTN_PROFILE_FUNCTION();

            auto& spriteData    = p_Pass.GetData<SpritePassData>();

            Batch& currentBatch = spriteData.Batches.NewBatch();
            currentBatch.Reset(s_Renderer->Resources.WhiteTexture);

            for (const auto& list : p_Pass.Lists)
            {
                for (const auto& renderCommand : list.GetCommands())
                {
                    if (!std::holds_alternative<SpriteCommand>(renderCommand.Command)) continue;

                    if (currentBatch.Instances.size() >= (size_t)MAX_INSTANCES || currentBatch.TextureSlot >= MAX_TEXTURE_SLOTS)
                    {
                        currentBatch = spriteData.Batches.NewBatch();
                        currentBatch.Reset(s_Renderer->Resources.WhiteTexture);
                    }

                    auto& command = std::get<SpriteCommand>(renderCommand.Command);

                    float textureIndex = 0.0f; // White texture
                    if (command.Texture)
                    {
                        for (uint32_t i = 1; i < currentBatch.TextureSlot; i++)
                        {
                            if (currentBatch.Textures[i]->Handle == command.Texture->Handle)
                            {
                                textureIndex = (float)i;
                                break;
                            }
                        }

                        if (textureIndex == 0.0f)
                        {
                            textureIndex = (float)currentBatch.TextureSlot;
                            currentBatch.Textures[currentBatch.TextureSlot] = command.Texture;
                            currentBatch.TextureSlot++;
                        }
                    }

                    InstanceData data = {};
                    data.Transform    = renderCommand.Transform;
                    data.Color        = command.Color;
                    data.UV           = { 0.0f, 0.0f, 1.0f, 1.0f };
                    data.Others.x     = command.Type == RenderType2D::Quad ? 0.0f : 1.0f; // Type
                    data.Others.y     = textureIndex;                                     // Texture Index
                    data.Others.z     = command.Thickness;                                // Thickness
                    data.Others.w     = command.Fade;                                     // Fade

                    if (command.Texture)
                    {
                        if (command.UseDirectUVs)
                            data.UV               = command.UV;
                        else
                        {
                            auto scale            = command.Scale;
                            auto offset           = command.Offset;
                            auto texSize          = glm::vec2(  command.Texture->GetWidth(), command.Texture->GetHeight() );

                            glm::vec2 spriteSize  = (command.Size == glm::vec2(0)) ? texSize  : command.Size;
                            glm::vec2 tile        = { scale.x == 0 ? 1.0f : scale.x,  scale.y == 0 ? 1.0f : scale.y };

                            glm::vec2 pixelOffset = command.BySize ? offset * spriteSize : offset;
                            glm::vec2 pixelSize   = tile * spriteSize;

                            glm::vec2 min         = pixelOffset / texSize;
                            glm::vec2 max         = (pixelOffset + pixelSize) / texSize;

                            data.UV               = glm::vec4(min, max);
                        }
                    }

                    currentBatch.Instances.push_back(data);

                    if (p_Pass.Info.Picking)
                        currentBatch.PickingIDs.push_back(renderCommand.ID);
                }
            }
        }

        void SpriteRenderer::Draw(RenderPass& p_Pass)
        {
            KTN_PROFILE_FUNCTION();

            auto& spriteData            = p_Pass.GetData<SpritePassData>();
            if (spriteData.Batches.Empty())
                return;

            PipelineSpecification pspec = {};
            pspec.pShader               = m_Resources.MainShader;
            pspec.TransparencyEnabled   = false;
            pspec.ColorTargets[0]       = p_Pass.Targets->Color;
            pspec.DepthTarget           = p_Pass.Targets->Depth;
            pspec.ResolveTexture        = p_Pass.Targets->Resolve;
            pspec.Samples               = p_Pass.Info.Samples;
            pspec.ClearTargets          = false;
            pspec.ClearColor            = p_Pass.Info.ClearColor;
            pspec.DebugName             = "SpriteMainPipeline";

            auto pipeline               = Pipeline::Get(pspec);

            auto commandBuffer          = RendererCommand::GetCurrentCommandBuffer();
            auto vp                     = p_Pass.Info.Projection * p_Pass.Info.View;

            pipeline->Begin(commandBuffer);

            RendererCommand::SetViewport(0.0f, 0.0f, p_Pass.Info.Width, p_Pass.Info.Height);

            for (auto& batch : spriteData.Batches)
            {
                m_Resources.MainSet->SetUniform("Camera", "u_ViewProjection", &vp);
                m_Resources.MainSet->Upload(commandBuffer);

                m_Resources.MainSet->SetUniform("u_Instances", "Instances", batch.Instances.data(), batch.Instances.size() * sizeof(InstanceData));
                m_Resources.MainSet->Upload(commandBuffer);

                m_Resources.MainSet->SetTexture("u_Textures", batch.Textures.data(), batch.TextureSlot);
                m_Resources.MainSet->Upload(commandBuffer);

                DrawElementsIndirectCommand command      = { 6, (uint32_t)batch.Instances.size(), 0, 0, 0 };
                m_Resources.IndirectBuffer->SetData(&command, sizeof(command));

                commandBuffer->BindSets(&m_Resources.MainSet);
                RendererCommand::DrawIndexedIndirect(DrawType::TRIANGLES, m_Resources.VAO, m_Resources.IndirectBuffer);

                Engine::Get().GetStats().DrawCalls      += 1;
                Engine::Get().GetStats().TrianglesCount += (uint32_t)batch.Instances.size() * 2;
            }

            pipeline->End(commandBuffer);


            if (p_Pass.Info.Picking)
            {
                PipelineSpecification pspec = {};
                pspec.pShader               = m_Resources.PickingShader;
                pspec.TransparencyEnabled   = false;
                pspec.ColorTargets[0]       = p_Pass.Targets->Picking;
                pspec.DepthTarget           = p_Pass.Targets->PickingDepth;
                pspec.DepthWrite            = false;
                pspec.Samples               = 1;
                pspec.ClearTargets          = false;
                pspec.ClearColor            = p_Pass.Info.ClearColor;
                pspec.DebugName             = "SpritePickingPipeline";

                auto pickingPipeline        = Pipeline::Get(pspec);

                pickingPipeline->Begin(commandBuffer);

                RendererCommand::SetViewport(0.0f, 0.0f, p_Pass.Info.Width, p_Pass.Info.Height);

                for (auto& batch : spriteData.Batches)
                {
                    m_Resources.PickingSet->SetUniform("Camera", "u_ViewProjection", &vp);
                    m_Resources.PickingSet->Upload(commandBuffer);

                    m_Resources.PickingSet->SetUniform("u_Instances", "Instances", batch.Instances.data(), batch.Instances.size() * sizeof(InstanceData));
                    m_Resources.PickingSet->Upload(commandBuffer);

                    PickingID count         = static_cast<PickingID>(batch.PickingIDs.size());
                    size_t bufferSize       = sizeof(PickingID) + sizeof(PickingID) * batch.PickingIDs.size();
                    m_Resources.PickingSet->PrepareStorageBuffer("PickingBuffer", bufferSize);
                    m_Resources.PickingSet->SetStorage("PickingBuffer", "Count", &count, sizeof(PickingID));
                    m_Resources.PickingSet->SetStorage("PickingBuffer", "PickingIDs", batch.PickingIDs.data(), sizeof(PickingID) * batch.PickingIDs.size());
                    m_Resources.PickingSet->Upload(commandBuffer);

                    commandBuffer->BindSets(&m_Resources.PickingSet);
                    RendererCommand::DrawIndexedIndirect(DrawType::TRIANGLES, m_Resources.VAO, m_Resources.IndirectBuffer);

                    Engine::Get().GetStats().DrawCalls += 1;
                }
                pickingPipeline->End(commandBuffer);
            }
        }

        #pragma endregion

        #pragma region LineRenderer

        void LineRenderer::Init()
        {
            KTN_PROFILE_FUNCTION();

            TaskManager::Get().AddTask({
                "Line::Init Primitive",
                TaskManager::Phase::Init,
                3,
                [this]()
                {
                    auto spirvSource = Shader::CompileOrGetSpirv("Assets/Shaders/PrimitiveLine.glsl");
                    KTN_CORE_INFO("Compiled PrimitiveLine shader!");
                    Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                    {
                        m_Resources.PrimitiveShader = Shader::Create(source);
                        m_Resources.PrimitiveSet = DescriptorSet::Create({ 0, m_Resources.PrimitiveShader });
                    });
                },
                true,
                TaskManager::SyncPoint::None
            });

            TaskManager::Get().AddTask({
                "Line::Init NonPrimitive",
                TaskManager::Phase::Init,
                4,
                [this]()
                {
                    auto spirvSource = Shader::CompileOrGetSpirv("Assets/Shaders/NonPrimitiveLine.glsl");
                    KTN_CORE_INFO("Compiled NonPrimitiveLine shader!");
                    Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                    {
                        m_Resources.NonPrimitiveShader = Shader::Create(source);
                        m_Resources.NonPrimitiveSet = DescriptorSet::Create({ 0, m_Resources.NonPrimitiveShader });
                    });
                },
                true,
                TaskManager::SyncPoint::None
            });

            m_Resources.IndirectBuffer = IndirectBuffer::Create(sizeof(DrawElementsIndirectCommand));
        }

        void LineRenderer::Begin()
        {
            KTN_PROFILE_FUNCTION();

        }

        void LineRenderer::Render(RenderPass& p_Pass)
        {
            KTN_PROFILE_FUNCTION();

            Build(p_Pass);

            Draw(p_Pass);
        }

        void LineRenderer::Build(RenderPass& p_Pass)
        {
            KTN_PROFILE_FUNCTION();

            auto& lineData             = p_Pass.GetData<LinePassData>();

            for (const auto& list : p_Pass.Lists)
            {
                for (const auto& renderCommand : list.GetCommands())
                {
                    if (!std::holds_alternative<LineCommand>(renderCommand.Command)) continue;

                    auto& command      = std::get<LineCommand>(renderCommand.Command);
                    auto& batches      = command.Primitive ? lineData.PrimitiveBatches[command.Width] : lineData.NonPrimitiveBatches;
                    auto& batch        = batches.GetOrCreateBatch();

                    if (batch.Instances.size() >= (size_t)MAX_INSTANCES)
                    {
                        batch          = batches.NewBatch();
                    }

                    InstanceData& data = batch.Instances.emplace_back();
                    data.Transform     = renderCommand.Transform;
                    data.Start         = glm::vec4(command.Start, 1.0f);
                    data.End           = glm::vec4(command.End, 1.0f);
                    data.Color         = command.Color;
                    data.Width         = command.Width;
                }
            }

        }

        void LineRenderer::Draw(RenderPass& p_Pass)
        {
            KTN_PROFILE_FUNCTION();

            auto& lineData              = p_Pass.GetData<LinePassData>();
            if (lineData.PrimitiveBatches.empty() && lineData.NonPrimitiveBatches.Empty()) return;

            PipelineSpecification pspec = {};
            pspec.pShader               = m_Resources.PrimitiveShader;
            pspec.TransparencyEnabled   = false;
            pspec.ColorTargets[0]       = p_Pass.Targets->Color;
            pspec.DepthTarget           = p_Pass.Targets->Depth;
            pspec.ResolveTexture        = p_Pass.Targets->Resolve;
            pspec.Samples               = p_Pass.Info.Samples;
            pspec.ClearTargets          = false;
            pspec.ClearColor            = p_Pass.Info.ClearColor;
            
            auto commandBuffer          = RendererCommand::GetCurrentCommandBuffer();
            auto vp                     = p_Pass.Info.Projection * p_Pass.Info.View;

            for (auto& [width, batches] : lineData.PrimitiveBatches)
            {
                pspec.LineWidth         = width;
                pspec.DebugName         = "PrimitivePipeline Width: " + std::to_string(width);

                auto pipeline           = Pipeline::Get(pspec);
                pipeline->Begin(commandBuffer);

                RendererCommand::SetViewport(0.0f, 0.0f, p_Pass.Info.Width, p_Pass.Info.Height);

                for (auto& batch : batches)
                {
                    m_Resources.PrimitiveSet->SetUniform("Camera", "u_ViewProjection", &vp);
                    m_Resources.PrimitiveSet->Upload(commandBuffer);

                    m_Resources.PrimitiveSet->SetUniform("u_Instances", "Instances", batch.Instances.data(), batch.Instances.size() * sizeof(InstanceData));
                    m_Resources.PrimitiveSet->Upload(commandBuffer);

                    DrawIndirectCommand command = {
                        2, (uint32_t)batch.Instances.size(), 0, 0
                    };
                    m_Resources.IndirectBuffer->SetData(&command, sizeof(command));

                    commandBuffer->BindSets(&m_Resources.PrimitiveSet);
                    RendererCommand::DrawIndirect(DrawType::LINES, nullptr, m_Resources.IndirectBuffer);

                    Engine::Get().GetStats().DrawCalls += 1;
                }

                pipeline->End(commandBuffer);
            }

            pspec.pShader               = m_Resources.NonPrimitiveShader;
            pspec.DebugName             = "NonPrimitivePipeline";

            auto pipeline               = Pipeline::Get(pspec);
            pipeline->Begin(commandBuffer);

            RendererCommand::SetViewport(0.0f, 0.0f, p_Pass.Info.Width, p_Pass.Info.Height);

            for (auto& batch : lineData.NonPrimitiveBatches)
            {
                m_Resources.NonPrimitiveSet->SetUniform("Camera", "u_ViewProjection", &vp);
                m_Resources.NonPrimitiveSet->Upload(commandBuffer);

                m_Resources.NonPrimitiveSet->SetUniform("u_Instances", "Instances", batch.Instances.data(), batch.Instances.size() * sizeof(InstanceData));
                m_Resources.NonPrimitiveSet->Upload(commandBuffer);

                DrawIndirectCommand command = {
                    2, (uint32_t)batch.Instances.size(), 0, 0
                };
                m_Resources.IndirectBuffer->SetData(&command, sizeof(command));

                commandBuffer->BindSets(&m_Resources.NonPrimitiveSet);
                RendererCommand::DrawIndirect(DrawType::LINES, nullptr, m_Resources.IndirectBuffer);

                Engine::Get().GetStats().DrawCalls += 1;
            }

            pipeline->End(commandBuffer);
        }

        #pragma endregion

        #pragma region TextRenderer

        void TextRenderer::Init()
        {
            KTN_PROFILE_FUNCTION();

            TaskManager::Get().AddTask({
                "Text::Init",
                TaskManager::Phase::Init,
                5,
                [this]()
                {
                    auto spirvSource           = Shader::CompileOrGetSpirv("Assets/Shaders/RenderText.glsl");
                    KTN_CORE_INFO("Compiled RenderText shader!");
                    Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                    {
                        m_Resources.MainShader = Shader::Create(source);
                        m_Resources.MainSet    = DescriptorSet::Create({ 0, m_Resources.MainShader });
                    });
                },
                true,
                TaskManager::SyncPoint::None
            });

            if (Engine::Get().GetSettings().MousePicking)
            {
                TaskManager::Get().AddTask({
                    "Text::Init MousePicking",
                    TaskManager::Phase::Init,
                    6,
                    [this]()
                    {
                        auto spirvSource              = Shader::CompileOrGetSpirv("Assets/Shaders/PickingText.glsl");
                        KTN_CORE_INFO("Compiled PickingText shader!");
                        Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                        {
                            m_Resources.PickingShader = Shader::Create(source);
                            m_Resources.PickingSet    = DescriptorSet::Create({ 0, m_Resources.PickingShader });
                        });
                    },
                    true,
                    TaskManager::SyncPoint::None
                    });
            }

            m_Resources.IndirectBuffer = IndirectBuffer::Create(sizeof(DrawElementsIndirectCommand));
        }

        void TextRenderer::Begin()
        {
            KTN_PROFILE_FUNCTION();

        }

        void TextRenderer::Render(RenderPass& p_Pass)
        {
            KTN_PROFILE_FUNCTION();

            Build(p_Pass);

            Draw(p_Pass);
        }

        void TextRenderer::Build(RenderPass& p_Pass)
        {
            KTN_PROFILE_FUNCTION();

            auto& textData = p_Pass.GetData<TextPassData>();
            Batch& currentBatch = textData.Batches.NewBatch();
            currentBatch.Reset(s_Renderer->Resources.WhiteTexture);

            for (const auto& list : p_Pass.Lists)
            {
                for (const auto& renderCommand : list.GetCommands())
                {
                    if (!std::holds_alternative<TextCommand>(renderCommand.Command)) continue;

                    if (currentBatch.Instances.size() >= (size_t)MAX_INSTANCES || currentBatch.TextureSlot >= MAX_TEXTURE_SLOTS)
                    {
                        currentBatch = textData.Batches.NewBatch();
                        currentBatch.Reset(s_Renderer->Resources.WhiteTexture);
                    }

                    auto& command = std::get<TextCommand>(renderCommand.Command);

                    if (!command.Font)
                    {
                        KTN_CORE_ERROR("Font is null!");
                        return;
                    }

                    auto utf32String          = UTF8ToUTF32(command.Text);
                    auto positions            = command.Font->CalculatePositions(utf32String, command.LineSpacing, command.Kerning);

                    if (command.DrawBg && command.BgColor.a > 0.0f)
                    {
                        glm::vec2 minPos{ 0.0f }, maxPos{ 0.0f };
                        for (size_t i = 0; i < positions.size(); i++)
                        {
                            const auto& pos   = positions[i].first;

                            glm::vec2 quadMin = { pos.x, pos.y };
                            glm::vec2 quadMax = { pos.z, pos.w };

                            minPos            = i == 0 ? quadMin : glm::min(minPos, quadMin);
                            maxPos            = glm::max(maxPos, quadMax);
                        }

                        InstanceData bgData   = {};
                        bgData.Transform      = renderCommand.Transform;
                        bgData.Positions      = { minPos, maxPos };
                        bgData.Color          = command.BgColor;
                        bgData.BgColor        = command.BgColor;
                        bgData.UV             = { glm::vec2(0.0f), glm::vec2(1.0f) };
                        bgData.TexIndex       = 0.0f;

                        currentBatch.Instances.push_back(bgData);

                        if (p_Pass.Info.Picking)
                            currentBatch.PickingIDs.push_back(renderCommand.ID);
                    }

                    auto texture       = command.Font->GetAtlasTexture();
                    float textureIndex = 0.0f; // White texture
                    if (texture)
                    {
                        for (uint32_t i = 1; i < currentBatch.TextureSlot; i++)
                        {
                            if (currentBatch.FontAtlasTextures[i]->Handle == texture->Handle)
                            {
                                textureIndex = (float)i;
                                break;
                            }
                        }

                        if (textureIndex == 0.0f)
                        {
                            textureIndex                                             = (float)currentBatch.TextureSlot;
                            currentBatch.FontAtlasTextures[currentBatch.TextureSlot] = texture;
                            currentBatch.TextureSlot++;
                        }
                    }

                    for (const auto& [pos, uvs] : positions)
                    {
                        glm::vec2 texCoordMin(uvs.x, uvs.y);
                        glm::vec2 texCoordMax(uvs.z, uvs.w);

                        float texelWidth  = 1.0f / texture->GetWidth();
                        float texelHeight = 1.0f / texture->GetHeight();
                        texCoordMin      *= glm::vec2(texelWidth, texelHeight);
                        texCoordMax      *= glm::vec2(texelWidth, texelHeight);

                        InstanceData data = {};
                        data.Transform    = renderCommand.Transform;
                        data.Positions    = pos;
                        data.Color        = command.Color;
                        data.BgColor      = command.CharBgColor;
                        data.UV           = { texCoordMin, texCoordMax };
                        data.TexIndex     = textureIndex;

                        currentBatch.Instances.push_back(data);

                        if (p_Pass.Info.Picking)
                            currentBatch.PickingIDs.push_back(renderCommand.ID);
                    }
                }
            }
        }

        void TextRenderer::Draw(RenderPass& p_Pass)
        {
            KTN_PROFILE_FUNCTION();

            auto& textData              = p_Pass.GetData<TextPassData>();
            if (textData.Batches.Empty()) return;

            PipelineSpecification pspec = {};
            pspec.pShader               = m_Resources.MainShader;
            pspec.TransparencyEnabled   = true;
            pspec.BlendModes[0]         = BlendMode::SrcAlphaOneMinusSrcAlpha;
            pspec.ColorTargets[0]       = p_Pass.Targets->Color;
            pspec.DepthTarget           = p_Pass.Targets->Depth;
            pspec.ResolveTexture        = p_Pass.Targets->Resolve;
            pspec.Samples               = p_Pass.Info.Samples;
            pspec.ClearTargets          = false;
            pspec.ClearColor            = p_Pass.Info.ClearColor;
            pspec.DebugName             = "TextMainPipeline";

            auto pipeline               = Pipeline::Get(pspec);

            auto commandBuffer          = RendererCommand::GetCurrentCommandBuffer();
            auto vp                     = p_Pass.Info.Projection * p_Pass.Info.View;

            pipeline->Begin(commandBuffer);

            RendererCommand::SetViewport(0.0f, 0.0f, p_Pass.Info.Width, p_Pass.Info.Height);

            for (auto& batch : textData.Batches)
            {
                m_Resources.MainSet->SetUniform("Camera", "u_ViewProjection", &vp);
                m_Resources.MainSet->Upload(commandBuffer);

                m_Resources.MainSet->SetUniform("u_Instances", "Instances", batch.Instances.data(), batch.Instances.size() * sizeof(InstanceData));
                m_Resources.MainSet->Upload(commandBuffer);

                m_Resources.MainSet->SetTexture("u_FontAtlasTextures", batch.FontAtlasTextures.data(), (uint32_t)batch.FontAtlasTextures.size());
                m_Resources.MainSet->Upload(commandBuffer);

                DrawElementsIndirectCommand command = { 6, (uint32_t)batch.Instances.size(), 0, 0, 0 };
                m_Resources.IndirectBuffer->SetData(&command, sizeof(command));

                commandBuffer->BindSets(&m_Resources.MainSet);
                RendererCommand::DrawIndirect(DrawType::TRIANGLE_STRIP, nullptr, m_Resources.IndirectBuffer);

                Engine::Get().GetStats().DrawCalls      += 1;
                Engine::Get().GetStats().TrianglesCount += (uint32_t)batch.Instances.size() * 2;
            }

            pipeline->End(commandBuffer);

            if (p_Pass.Info.Picking)
            {
                PipelineSpecification pspec = {};
                pspec.pShader               = m_Resources.PickingShader;
                pspec.TransparencyEnabled   = false;
                pspec.ColorTargets[0]       = p_Pass.Targets->Picking;
                pspec.DepthTarget           = p_Pass.Targets->PickingDepth;
                pspec.DepthWrite            = false;
                pspec.ClearTargets          = false;
                pspec.Samples               = 1;
                pspec.DebugName             = "TextPickingPipeline";

                auto pickingPipeline        = Pipeline::Get(pspec);

                pickingPipeline->Begin(commandBuffer);

                RendererCommand::SetViewport(0.0f, 0.0f, p_Pass.Info.Width, p_Pass.Info.Height);

                for (auto& batch : textData.Batches)
                {
                    m_Resources.PickingSet->SetUniform("Camera", "u_ViewProjection", &vp);
                    m_Resources.PickingSet->Upload(commandBuffer);

                    m_Resources.PickingSet->SetUniform("u_Instances", "Instances", batch.Instances.data(), batch.Instances.size() * sizeof(InstanceData));
                    m_Resources.PickingSet->Upload(commandBuffer);

                    PickingID count         = static_cast<PickingID>(batch.PickingIDs.size());
                    size_t bufferSize       = sizeof(PickingID) + sizeof(PickingID) * batch.PickingIDs.size();
                    m_Resources.PickingSet->PrepareStorageBuffer("PickingBuffer", bufferSize);
                    m_Resources.PickingSet->SetStorage("PickingBuffer", "Count", &count, sizeof(PickingID));
                    m_Resources.PickingSet->SetStorage("PickingBuffer", "PickingIDs", batch.PickingIDs.data(), sizeof(PickingID) * batch.PickingIDs.size());
                    m_Resources.PickingSet->Upload(commandBuffer);

                    commandBuffer->BindSets(&m_Resources.PickingSet);
                    RendererCommand::DrawIndirect(DrawType::TRIANGLE_STRIP, nullptr, m_Resources.IndirectBuffer);

                    Engine::Get().GetStats().DrawCalls += 1;
                }

                pickingPipeline->End(commandBuffer);
            }
        }

        #pragma endregion

    } // namespace

} // namespace KTN