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



namespace KTN
{
    namespace
    {
        static constexpr uint16_t MaxInstances = 5000;
        static constexpr uint8_t MaxTextureSlots = 32;

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

        struct RenderData
        {
            Ref<Texture2D> RenderTarget       = nullptr;
            Ref<Texture2D> ResolveTexture     = nullptr;
            Ref<Texture2D> MainTexture        = nullptr;
            Ref<Texture2D> MainPickingTexture = nullptr;
            Ref<Texture2D> MainDepthTexture   = nullptr;
            uint32_t Width                    = 0;
            uint32_t Height                   = 0;
            glm::mat4 Projection              = { 1.0f };
            glm::mat4 View                    = { 1.0f };
            uint8_t Samples                   = 1;
            glm::vec4 ClearColor              = { 0.0f, 0.0f, 0.0f, 1.0f };

            Ref<Texture2D> WhiteTexture       = nullptr;

            Ref<Shader> FinalPassShader       = nullptr;
            Ref<DescriptorSet> FinalPassSet   = nullptr;
        };

        std::u32string UTF8ToUTF32(const std::string& p_UTF8)
        {
            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
            return converter.from_bytes(p_UTF8);
        }

    } // namespace

    namespace R2D
    {
        struct InstanceData
        {
            glm::mat4 Transform;
            glm::vec4 Color;
            glm::vec4 UV;
            glm::vec4 Others; // Type, TexIndex, Thickness, Fade
        };

        struct Data
        {
            Ref<Shader> MainShader        = nullptr;
            Ref<DescriptorSet> Set        = nullptr;
            Ref<Pipeline> MainPipeline    = nullptr;
            Ref<IndirectBuffer> Buffer    = nullptr;

            Ref<VertexArray> VAO          = nullptr;

            struct InstanceEntry
            {
                InstanceData Instance;
                int EntityID;
            };

            std::vector<InstanceEntry> InstanceEntries;

            std::vector<InstanceData> SortedInstances;
            std::vector<int> SortedEntityIDs;

            uint32_t TextureSlotIndex     = 1;
            std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;

            Ref<Shader> PickingShader     = nullptr;
            Ref<DescriptorSet> PickingSet = nullptr;

            std::mutex InstanceMutex;

            void Init();
            void Begin();
            void StartBatch();
            void FlushAndReset();
            void Flush();
            void Submit(const RenderCommand& p_Command);
        };

    } // namespace R2D

    namespace Line
    {
        struct InstanceData
        {
            glm::mat4 Transform;
            glm::vec4 Start;
            glm::vec4 End;
            glm::vec4 Color;
            alignas(16) float Width;
        };

        struct Data
        {
            Ref<Shader> PrimitiveShader        = nullptr;
            Ref<DescriptorSet> PrimitiveSet    = nullptr;

            Ref<Shader> NonPrimitiveShader     = nullptr;
            Ref<DescriptorSet> NonPrimitiveSet = nullptr;

            Ref<IndirectBuffer> Buffer         = nullptr;

            // instances per width
            std::unordered_map<float, std::pair<bool, std::vector<InstanceData>>> Instances;

            void Init();
            void Begin();
            void StartBatch();
            void FlushAndReset();
            void Flush();
            void Submit(const RenderCommand& p_Command);
        };
    } // namespace Line
    
    namespace Text 
    {
        struct InstanceData
        {
            glm::mat4 Transform;
            glm::vec4 Positions;
            glm::vec4 Color;
            glm::vec4 BgColor;
            glm::vec4 UV;
            alignas(16) float TexIndex;
        };

        struct Data
        {
            Ref<Shader> MainShader        = nullptr;
            Ref<DescriptorSet> Set        = nullptr;
            Ref<Pipeline> MainPipeline    = nullptr;
            Ref<IndirectBuffer> Buffer    = nullptr;

            std::vector<InstanceData> Instances;

            uint32_t TextureIndex = 1;
            std::array<Ref<Texture2D>, MaxTextureSlots> FontAtlasTextures;

            Ref<Shader> PickingShader     = nullptr;
            Ref<DescriptorSet> PickingSet = nullptr;
            EntityBufferData EntityBuffer = {};

            void Init();
            void Begin();
            void StartBatch();
            void FlushAndReset();
            void Flush();
        };
        
    } // Text

    static RenderData* s_Data     = nullptr;
    static R2D::Data* s_R2DData   = nullptr;
    static Line::Data* s_LineData = nullptr;
    static Text::Data* s_TextData = nullptr;

    void Renderer::Init()
    {
        KTN_PROFILE_FUNCTION();

        s_Data = new RenderData();

        uint32_t whiteTextureData = 0xffffffff;
        s_Data->WhiteTexture      = Texture2D::Create({}, (uint8_t*)&whiteTextureData, sizeof(uint32_t));

        TaskManager::Get().AddTask({
            "Renderer::Init",
            TaskManager::Phase::Init,
            0,
            []()
            {
                auto spirvSource = Shader::CompileOrGetSpirv("Assets/Shaders/FinalPass.glsl");
                Application::Get().SubmitToMainThread([source = std::move(spirvSource)]()
                {
                    s_Data->FinalPassShader = Shader::Create(source);
                    s_Data->FinalPassSet    = DescriptorSet::Create({ 0, s_Data->FinalPassShader });
                });
            },
            true,
            TaskManager::SyncPoint::None
         });

        // R2D
        {
            s_R2DData = new R2D::Data();
            s_R2DData->Init();
        }

        // Line
        {
            s_LineData = new Line::Data();
            s_LineData->Init();
        }

        // Text
        {
            s_TextData = new Text::Data();
            s_TextData->Init();
        }
    }

    void Renderer::Shutdown()
    {
        KTN_PROFILE_FUNCTION();

        delete s_Data;
        delete s_R2DData;
        delete s_LineData;
        delete s_TextData;
    }

    void Renderer::Clear()
    {
        KTN_PROFILE_FUNCTION();

        if (s_Data->MainTexture)
            RendererCommand::ClearRenderTarget(s_Data->MainTexture, s_Data->ClearColor);
        if (s_Data->MainPickingTexture)
            RendererCommand::ClearRenderTarget(s_Data->MainPickingTexture, -1);
        if (s_Data->ResolveTexture)
            RendererCommand::ClearRenderTarget(s_Data->ResolveTexture, s_Data->ClearColor);
        if (s_Data->MainDepthTexture)
            RendererCommand::ClearRenderTarget(s_Data->MainDepthTexture, 1);
    }

    void Renderer::Begin(const RenderBeginInfo& p_Info)
    {
        KTN_PROFILE_FUNCTION();

        s_Data->RenderTarget        = p_Info.RenderTarget;
        s_Data->Width               = p_Info.Width;
        s_Data->Height              = p_Info.Height;
        s_Data->Projection          = p_Info.Projection;
        s_Data->View                = p_Info.View;
        s_Data->Samples             = p_Info.Samples;

        TextureSpecification tspec  = {};
        tspec.Format                = TextureFormat::RGBA32_FLOAT;
        tspec.Usage                 = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
        tspec.Width                 = p_Info.Width;
        tspec.Height                = p_Info.Height;
        tspec.GenerateMips          = false;
        tspec.AnisotropyEnable      = false;
        tspec.Samples               = p_Info.Samples;
        tspec.DebugName             = "MainTexture";

        s_Data->MainTexture         = Texture2D::Get(tspec);

        if (Engine::Get().GetSettings().MousePicking)
        {
            tspec.Format               = TextureFormat::R32_INT;
            tspec.Samples              = 1;
            tspec.DebugName            = "MainPickingTexture";

            s_Data->MainPickingTexture = Texture2D::Get(tspec);
        }


        if (p_Info.Samples > 1)
        {
            tspec.Format           = TextureFormat::RGBA32_FLOAT;
            tspec.DebugName        = "MainResolveTexture";

            s_Data->ResolveTexture = Texture2D::Get(tspec);
        }

        tspec.Samples            = p_Info.Samples;
        tspec.Format             = TextureFormat::D32_FLOAT;
        tspec.Usage              = TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT;
        tspec.DebugName          = "MainDepthTexture";

        s_Data->MainDepthTexture = Texture2D::Get(tspec);

        if (p_Info.Clear)
            Clear();

        s_R2DData->Begin();
        s_LineData->Begin();
        s_TextData->Begin();

        TaskManager::Get().ExecutePhase(TaskManager::Phase::Render);
    }

    void Renderer::End()
    {
        KTN_PROFILE_FUNCTION();

        TaskManager::Get().WaitForSyncPoint(TaskManager::SyncPoint::FrameRender);

        auto commandBuffer = RendererCommand::GetCurrentCommandBuffer();

        s_R2DData->Flush();
        s_LineData->Flush();
        s_TextData->Flush();

        // Final Pass
        {
            PipelineSpecification pspec = {};
            pspec.ColorTargets[0]       = s_Data->RenderTarget;
            pspec.pShader               = s_Data->FinalPassShader;
            pspec.SwapchainTarget       = s_Data->RenderTarget == nullptr;
            pspec.ClearTargets          = true;
            pspec.DepthTest             = false;
            pspec.DepthWrite            = false;
            pspec.DebugName             = "FinalPassPipeline";

            auto pipeline               = Pipeline::Get(pspec);

            pipeline->Begin(commandBuffer);

            commandBuffer->SetViewport(0.0f, 0.0f, s_Data->Width, s_Data->Height);

            s_Data->FinalPassSet->SetTexture("u_Texture", s_Data->MainTexture);
            s_Data->FinalPassSet->Upload(commandBuffer);

            commandBuffer->BindSets(&s_Data->FinalPassSet);

            RendererCommand::Draw(DrawType::TRIANGLES, nullptr, 6);

            pipeline->End(commandBuffer);
        }
    }

    void Renderer::Submit(const RenderCommand& p_Command)
    {
        KTN_PROFILE_FUNCTION();

        if (p_Command.Type == RenderType::R2D)
            s_R2DData->Submit(p_Command);
        else if (p_Command.Type == RenderType::Line)
            s_LineData->Submit(p_Command);
        else
            KTN_CORE_ERROR("Unknown render type!");
    }

    void Renderer::SubmitString(const std::string& p_String, const Ref<DFFont>& p_Font, const glm::mat4& p_Transform, const TextParams& p_Params, int p_EntityID)
    {
        KTN_PROFILE_FUNCTION();

        if (!s_TextData)
        {
            KTN_CORE_ERROR("Something wrong! s_TextData is nullptr");
            return;
        }

        if (s_TextData->Instances.size() >= (size_t)MaxInstances)
            s_TextData->FlushAndReset();

        if (p_Font == nullptr)
        {
            KTN_CORE_ERROR("Font is null!");
            return;
        }

        auto utf32String = UTF8ToUTF32(p_String);
        auto positions   = p_Font->CalculatePositions(utf32String, p_Params.LineSpacing, p_Params.Kerning);

        if (p_Params.DrawBg && p_Params.BgColor.a > 0.0f)
        {
            glm::vec2 minPos{0.0f}, maxPos{0.0f};
            for (size_t i = 0; i < positions.size(); i++)
            {
                const auto& pos   = positions[i].first;

                glm::vec2 quadMin = { pos.x, pos.y };
                glm::vec2 quadMax = { pos.z, pos.w };

                minPos            = i == 0 ? quadMin : glm::min(minPos, quadMin);
                maxPos            = glm::max(maxPos, quadMax);
            }

            Text::InstanceData bgData = {};
            bgData.Transform          = p_Transform;
            bgData.Positions          = { minPos, maxPos };
            bgData.Color              = p_Params.BgColor;
            bgData.BgColor            = p_Params.BgColor;
            bgData.UV                 = { glm::vec2(0.0f), glm::vec2(1.0f) };
            bgData.TexIndex           = 0.0f;

            s_TextData->Instances.push_back(bgData);

            if (Engine::Get().GetSettings().MousePicking)
            {
                s_TextData->EntityBuffer.EntityIDS.push_back(p_EntityID);
                s_TextData->EntityBuffer.Count++;
            }
        }

        auto texture = p_Font->GetAtlasTexture();
        float textureIndex = 0.0f; // White texture
        if (texture)
        {
            for (uint32_t i = 1; i < s_TextData->TextureIndex; i++)
            {
                if (s_TextData->FontAtlasTextures[i]->Handle == texture->Handle)
                {
                    textureIndex = (float)i;
                    break;
                }
            }

            if (textureIndex == 0.0f)
            {
                if (s_TextData->TextureIndex >= MaxTextureSlots)
                    s_TextData->FlushAndReset();

                textureIndex = (float)s_TextData->TextureIndex;
                s_TextData->FontAtlasTextures[s_TextData->TextureIndex] = texture;
                s_TextData->TextureIndex++;
            }
        }

        for (const auto& [ pos, uvs ] : positions)
        {
            glm::vec2 texCoordMin( uvs.x, uvs.y );
            glm::vec2 texCoordMax( uvs.z, uvs.w );

            float texelWidth   = 1.0f / texture->GetWidth();
            float texelHeight  = 1.0f / texture->GetHeight();
            texCoordMin       *= glm::vec2(texelWidth, texelHeight);
            texCoordMax       *= glm::vec2(texelWidth, texelHeight);
            
            if (Engine::Get().GetSettings().MousePicking)
            {
                s_TextData->EntityBuffer.EntityIDS.push_back(p_EntityID);
                s_TextData->EntityBuffer.Count++;
            }

            Text::InstanceData data = {};
            data.Transform          = p_Transform;
            data.Positions          = pos;
            data.Color              = p_Params.Color;
            data.BgColor            = p_Params.CharBgColor;
            data.UV                 = { texCoordMin, texCoordMax };
            data.TexIndex           = textureIndex;

            s_TextData->Instances.push_back(data);
        }
    }

    Ref<Texture2D> Renderer::GetPickingTexture()
    {
        return s_Data->MainPickingTexture;
    }

    namespace R2D
    {
        void Data::Init()
        {
            KTN_PROFILE_FUNCTION();

            TaskManager::Get().AddTask({
                "R2D::Init",
                TaskManager::Phase::Init,
                1,
                [this]()
                {
                    auto spirvSource = Shader::CompileOrGetSpirv("Assets/Shaders/R2D_Shader.glsl");
                    KTN_CORE_INFO("Compiled R2D_Shader shader!");
                    Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                    {
                        MainShader   = Shader::Create(source);
                        Set          = DescriptorSet::Create({ 0, MainShader });
                    });
                },
                true,
                TaskManager::SyncPoint::None
            });

            if (Engine::Get().GetSettings().MousePicking)
            {
                TaskManager::Get().AddTask({
                    "R2D::Init MousePicking",
                    TaskManager::Phase::Init,
                    2,
                    [this]()
                    {
                        auto spirvSource = Shader::CompileOrGetSpirv("Assets/Shaders/R2D_Picking.glsl");
                        KTN_CORE_INFO("Compiled R2D_Picking shader!");
                        Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                        {
                            PickingShader = Shader::Create(source);
                            PickingSet = DescriptorSet::Create({ 0, PickingShader });
                        });
                    },
                    true,
                    TaskManager::SyncPoint::None
                });
            }

            TaskManager::Get().AddTask({
                "R2D::SortInstances",
                TaskManager::Phase::Render,
                0,
                [this]()
                {
                    while (true)
                    {
                        auto syncPoint = TaskManager::Get().CurrentSyncPoint();

                        if (InstanceEntries.empty())
                        {
                            if (syncPoint == TaskManager::SyncPoint::None) continue;
                            else if (syncPoint == TaskManager::SyncPoint::FrameRender) break;
                        }

                        std::vector<InstanceEntry> entriesCopy;
                        {
                            std::scoped_lock lock(InstanceMutex);
                            entriesCopy = InstanceEntries;
                        }

                        std::vector<uint32_t> sortedIndices(entriesCopy.size());
                        std::iota(sortedIndices.begin(), sortedIndices.end(), 0);

                        std::sort(sortedIndices.begin(), sortedIndices.end(),
                        [&](uint32_t a, uint32_t b)
                        {
                            return entriesCopy[a].Instance.Transform[3].y > entriesCopy[b].Instance.Transform[3].y;
                        });

                        bool mousePicking = Engine::Get().GetSettings().MousePicking;

                        std::vector<InstanceData> localSortedInstances;
                        std::vector<int> localSortedEntityIDs;

                        localSortedInstances.resize(sortedIndices.size());
                        if (mousePicking)
                            localSortedEntityIDs.resize(sortedIndices.size());

                        for (size_t i = 0; i < sortedIndices.size(); i++)
                        {
                            auto& entry = entriesCopy[sortedIndices[i]];
                            localSortedInstances[i] = entry.Instance;
                            if (mousePicking)
                                localSortedEntityIDs[i] = entry.EntityID;
                        }

                        {
                            std::scoped_lock lock(InstanceMutex);
                            SortedInstances = std::move(localSortedInstances);
                            SortedEntityIDs = std::move(localSortedEntityIDs);
                        }

                        if (syncPoint == TaskManager::SyncPoint::FrameRender) break;
                    }
                },
                true,
                TaskManager::SyncPoint::FrameRender
            });

            VAO              = VertexArray::Create();

            float vertices[] = {
                // positions
                -0.5f, -0.5f, 0.0f,
                 0.5f, -0.5f, 0.0f,
                 0.5f,  0.5f, 0.0f,
                -0.5f,  0.5f, 0.0f
            };

            auto vbo         = VertexBuffer::Create(vertices, sizeof(vertices));
            vbo->SetLayout({
                { DataType::Float3 , "a_Position"    }
            });
            VAO->SetVertexBuffer(vbo);

            uint32_t indices[] = {
                0, 1, 3, // first triangle
                1, 2, 3  // second triangle
            };
            auto ebo           = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
            VAO->SetIndexBuffer(ebo);

            TextureSlots.fill(s_Data->WhiteTexture);

            Buffer             = IndirectBuffer::Create(sizeof(DrawElementsIndirectCommand));
        }

        void Data::Begin()
        {
            KTN_PROFILE_FUNCTION();

            PipelineSpecification pspec = {};
            pspec.pShader               = MainShader;
            pspec.TransparencyEnabled   = false;
            pspec.ColorTargets[0]       = s_Data->MainTexture;
            pspec.DepthTarget           = s_Data->MainDepthTexture;
            pspec.ClearTargets          = false;
            pspec.ResolveTexture        = s_Data->ResolveTexture;
            pspec.Samples               = s_Data->Samples;
            pspec.DebugName             = "R2DMainPipeline";

            MainPipeline = Pipeline::Get(pspec);

            StartBatch();
        }

        void Data::StartBatch()
        {
            KTN_PROFILE_FUNCTION();

            std::scoped_lock lock(InstanceMutex);

            InstanceEntries.clear();
            TextureSlotIndex = 1;
            SortedInstances.clear();
            SortedEntityIDs.clear();
        }

        void Data::FlushAndReset()
        {
            KTN_PROFILE_FUNCTION();

            Flush();
            StartBatch();
        }

        void Data::Flush()
        {
            KTN_PROFILE_FUNCTION();

            auto commandBuffer = RendererCommand::GetCurrentCommandBuffer();
            auto vp            = s_Data->Projection * s_Data->View;

            if (!InstanceEntries.empty())
            {
                MainPipeline->Begin(commandBuffer);

                RendererCommand::SetViewport(0.0f, 0.0f, s_Data->Width, s_Data->Height);

                Set->SetUniform("Camera", "u_ViewProjection", &vp);
                Set->Upload(commandBuffer);

                Set->SetUniform("u_Instances", "Instances", SortedInstances.data(), SortedInstances.size() * sizeof(InstanceData));
                Set->Upload(commandBuffer);

                Set->SetTexture("u_Textures", TextureSlots.data(), TextureSlotIndex);
                Set->Upload(commandBuffer);

                DrawElementsIndirectCommand command = {
                    6, (uint32_t)SortedInstances.size(), 0, 0, 0
                };
                Buffer->SetData(&command, sizeof(command));

                commandBuffer->BindSets(&Set);
                RendererCommand::DrawIndexedIndirect(DrawType::TRIANGLES, VAO, Buffer);

                Engine::Get().GetStats().DrawCalls      += 1;
                Engine::Get().GetStats().TrianglesCount += (uint32_t)SortedInstances.size() * 2;

                MainPipeline->End(commandBuffer);

                if (Engine::Get().GetSettings().MousePicking)
                {
                    PipelineSpecification pspec = {};
                    pspec.pShader               = PickingShader;
                    pspec.TransparencyEnabled   = false;
                    pspec.ColorTargets[0]       = s_Data->MainPickingTexture;
                    pspec.DepthTarget           = s_Data->MainDepthTexture;
                    pspec.DepthWrite            = false;
                    pspec.ClearTargets          = false;
                    pspec.Samples               = 1;
                    pspec.DebugName             = "R2DPickingPipeline";

                    auto pipeline               = Pipeline::Get(pspec);

                    pipeline->Begin(commandBuffer);

                    RendererCommand::SetViewport(0.0f, 0.0f, s_Data->Width, s_Data->Height);

                    PickingSet->SetUniform("Camera", "u_ViewProjection", &vp);
                    PickingSet->Upload(commandBuffer);

                    PickingSet->SetUniform("u_Instances", "Instances", SortedInstances.data(), SortedInstances.size() * sizeof(InstanceData));
                    PickingSet->Upload(commandBuffer);

                    int count = SortedEntityIDs.size();

                    size_t bufferSize = sizeof(int) + sizeof(int) * SortedEntityIDs.size();
                    PickingSet->PrepareStorageBuffer("EntityBuffer", bufferSize);
                    PickingSet->SetStorage("EntityBuffer", "Count", &count, sizeof(int));
                    PickingSet->SetStorage("EntityBuffer", "EnttIDs", SortedEntityIDs.data(), sizeof(int) * SortedEntityIDs.size());
                    PickingSet->Upload(commandBuffer);

                    commandBuffer->BindSets(&PickingSet);
                    RendererCommand::DrawIndexedIndirect(DrawType::TRIANGLES, VAO, Buffer);

                    Engine::Get().GetStats().DrawCalls += 1;

                    pipeline->End(commandBuffer);
                }
            }
        }
            
        void Data::Submit(const RenderCommand& p_Command)
        {
            KTN_PROFILE_FUNCTION();

            if (InstanceEntries.size() >= (size_t)MaxInstances)
                FlushAndReset();

            std::scoped_lock lock(InstanceMutex);

            auto& entry    = InstanceEntries.emplace_back();
            entry.EntityID = p_Command.EntityID;

            float textureIndex = 0.0f; // White texture
            if (p_Command.Render2D.Texture)
            {
                for (uint32_t i = 1; i < TextureSlotIndex; i++)
                {
                    if (TextureSlots[i]->Handle == p_Command.Render2D.Texture->Handle)
                    {
                        textureIndex = (float)i;
                        break;
                    }
                }

                if (textureIndex == 0.0f)
                {
                    if (TextureSlotIndex >= MaxTextureSlots)
                        FlushAndReset();

                    textureIndex = (float)TextureSlotIndex;
                    TextureSlots[TextureSlotIndex] = p_Command.Render2D.Texture;
                    TextureSlotIndex++;
                }
            }

            InstanceData data = {};
            data.Transform    = p_Command.Transform;
            data.Color        = p_Command.Render2D.Color;
            data.UV           = { 0.0f, 0.0f, 1.0f, 1.0f };
            data.Others.x     = p_Command.Render2D.Type == RenderType2D::Quad ? 0.0f : 1.0f; // Type
            data.Others.y     = textureIndex; // Texture Index
            data.Others.z     = p_Command.Render2D.Thickness; // Thickness
            data.Others.w     = p_Command.Render2D.Fade; // Fade

            if (p_Command.Render2D.Texture)
            {
                if (p_Command.Render2D.UseDirectUVs)
                    data.UV               = p_Command.Render2D.UV;
                else
                {
                    auto scale            = p_Command.Render2D.Scale;
                    auto offset           = p_Command.Render2D.Offset;

                    auto texSize          = glm::vec2(
                        p_Command.Render2D.Texture->GetWidth(),
                        p_Command.Render2D.Texture->GetHeight()
                    );

                    glm::vec2 spriteSize  = (p_Command.Render2D.Size == glm::vec2(0))
                        ? texSize
                        : p_Command.Render2D.Size;

                    glm::vec2 tile        = {
                        scale.x == 0 ? 1.0f : scale.x,
                        scale.y == 0 ? 1.0f : scale.y
                    };

                    glm::vec2 pixelOffset = p_Command.Render2D.BySize
                        ? offset * spriteSize
                        : offset;

                    glm::vec2 pixelSize   = tile * spriteSize;

                    glm::vec2 min         = pixelOffset / texSize;
                    glm::vec2 max         = (pixelOffset + pixelSize) / texSize;

                    data.UV               = glm::vec4(min, max);
                }
            }

            entry.Instance = data;
        }
    
    } // namespace R2D

    namespace Line
    {
        void Data::Init()
        {
            KTN_PROFILE_FUNCTION();

            TaskManager::Get().AddTask({
                "Line::Init",
                TaskManager::Phase::Init,
                3,
                [this]()
                {
                    auto spirvSource    = Shader::CompileOrGetSpirv("Assets/Shaders/PrimitiveLine.glsl");
                    KTN_CORE_INFO("Compiled PrimitiveLine shader!");
                    Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                    {
                        PrimitiveShader = Shader::Create(source);
                        PrimitiveSet    = DescriptorSet::Create({ 0, PrimitiveShader });
                    });
                },
                true,
                TaskManager::SyncPoint::None
            });

            TaskManager::Get().AddTask({
                "Line::Init MousePicking",
                TaskManager::Phase::Init,
                4,
                [this]()
                {
                    auto spirvSource       = Shader::CompileOrGetSpirv("Assets/Shaders/NonPrimitiveLine.glsl");
                    KTN_CORE_INFO("Compiled NonPrimitiveLine shader!");
                    Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                    {
                        NonPrimitiveShader = Shader::Create(source);
                        NonPrimitiveSet    = DescriptorSet::Create({ 0, PrimitiveShader });
                    });
                },
                true,
                TaskManager::SyncPoint::None
            });


            //PrimitiveShader    = Shader::Create("Assets/Shaders/PrimitiveLine.glsl");
            //PrimitiveSet       = DescriptorSet::Create({ 0, PrimitiveShader });

            //NonPrimitiveShader = Shader::Create("Assets/Shaders/NonPrimitiveLine.glsl");
            //NonPrimitiveSet    = DescriptorSet::Create({ 0, NonPrimitiveShader });

            Buffer             = IndirectBuffer::Create(sizeof(DrawElementsIndirectCommand));
        }

        void Data::Begin()
        {
            KTN_PROFILE_FUNCTION();

            StartBatch();
        }

        void Data::StartBatch()
        {
            KTN_PROFILE_FUNCTION();

            Instances.clear();
        }

        void Data::FlushAndReset()
        {
            KTN_PROFILE_FUNCTION();

            Flush();
            StartBatch();
        }

        void Data::Flush()
        {
            KTN_PROFILE_FUNCTION();

            if (Instances.empty())
                return;

            auto commandBuffer = RendererCommand::GetCurrentCommandBuffer();
            auto vp            = s_Data->Projection * s_Data->View;

            for (auto& [width, data] : Instances)
            {
                auto& [primitive, instances] = data;
                if (instances.empty())
                    continue;

                PipelineSpecification pspec = {};
                pspec.pShader               = primitive ? PrimitiveShader : NonPrimitiveShader;
                pspec.TransparencyEnabled   = false;
                pspec.ColorTargets[0]       = s_Data->MainTexture;
                pspec.DepthTarget           = s_Data->MainDepthTexture;
                pspec.ClearTargets          = false;
                pspec.ResolveTexture        = s_Data->ResolveTexture;
                pspec.Samples               = 1;
                pspec.LineWidth             = primitive ? width : 1.0f;
                pspec.DepthWrite            = false;
                pspec.DepthTest             = false;
                pspec.DebugName             = std::string(primitive ? "Primitive" : "NonPrimitive") + "LineMainPipeline";

                auto pipeline               = Pipeline::Get(pspec);

                pipeline->Begin(commandBuffer);

                RendererCommand::SetViewport(0.0f, 0.0f, s_Data->Width, s_Data->Height);

                auto& set = primitive ? PrimitiveSet : NonPrimitiveSet;

                set->SetUniform("Camera", "u_ViewProjection", &vp);
                set->Upload(commandBuffer);
        
                set->SetUniform("u_Instances", "Instances", instances.data(), instances.size() * sizeof(InstanceData));
                set->Upload(commandBuffer);

                DrawIndirectCommand command = {
                    2, (uint32_t)instances.size(), 0, 0
                };
                Buffer->SetData(&command, sizeof(command));

                commandBuffer->BindSets(&set);
                RendererCommand::DrawIndirect(DrawType::LINES, nullptr, Buffer);

                Engine::Get().GetStats().DrawCalls += 1;

                pipeline->End(commandBuffer);
            }
        }

        void Data::Submit(const RenderCommand& p_Command)
        {
            KTN_PROFILE_FUNCTION();

            if (Instances.size() >= (size_t)MaxInstances)
                FlushAndReset();

            InstanceData data = {};
            data.Transform    = p_Command.Transform;
            data.Start        = glm::vec4(p_Command.Line.Start, 1.0f);
            data.End          = glm::vec4(p_Command.Line.End, 1.0f);
            data.Color        = p_Command.Line.Color;
            data.Width        = p_Command.Line.Width;

            Instances[p_Command.Line.Width].first = p_Command.Line.Primitive;
            Instances[p_Command.Line.Width].second.push_back(data);
        }

    } // namespace Line

    namespace Text
    {
        void Data::Init()
        {
            KTN_PROFILE_FUNCTION();

            TaskManager::Get().AddTask({
                "Text::Init",
                TaskManager::Phase::Init,
                5,
                [this]()
                {
                    auto spirvSource = Shader::CompileOrGetSpirv("Assets/Shaders/RenderText.glsl");
                    KTN_CORE_INFO("Compiled RenderText shader!");
                    Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                    {
                        MainShader   = Shader::Create(source);
                        Set          = DescriptorSet::Create({ 0, MainShader });
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
                        auto spirvSource  = Shader::CompileOrGetSpirv("Assets/Shaders/PickingText.glsl");
                        KTN_CORE_INFO("Compiled PickingText shader!");
                        Application::Get().SubmitToMainThread([this, source = std::move(spirvSource)]()
                        {
                            PickingShader = Shader::Create(source);
                            PickingSet    = DescriptorSet::Create({ 0, PickingShader });
                        });
                    },
                    true,
                    TaskManager::SyncPoint::None
                });
            }

            FontAtlasTextures.fill(s_Data->WhiteTexture);

            Buffer = IndirectBuffer::Create(sizeof(DrawElementsIndirectCommand));
        }

        void Data::Begin()
        {
            KTN_PROFILE_FUNCTION();

            PipelineSpecification pspec = {};
            pspec.pShader               = MainShader;
            pspec.TransparencyEnabled   = true;
            pspec.BlendModes[0]         = BlendMode::SrcAlphaOneMinusSrcAlpha;
            pspec.ColorTargets[0]       = s_Data->MainTexture;
            pspec.DepthTarget           = s_Data->MainDepthTexture;
            pspec.ClearTargets          = false;
            pspec.ResolveTexture        = s_Data->ResolveTexture;
            pspec.Samples               = s_Data->Samples;
            pspec.DebugName             = "TextMainPipeline";

            MainPipeline                = Pipeline::Get(pspec);

            StartBatch();
        }

        void Data::StartBatch()
        {
            KTN_PROFILE_FUNCTION();

            Instances.clear();
            TextureIndex = 1;

            EntityBuffer.Count = 0;
            EntityBuffer.EntityIDS.clear();
        }

        void Data::FlushAndReset()
        {
            KTN_PROFILE_FUNCTION();

            Flush();
            StartBatch();
        }

        void Data::Flush()
        {
            KTN_PROFILE_FUNCTION();

            auto commandBuffer = RendererCommand::GetCurrentCommandBuffer();
            auto vp            = s_Data->Projection * s_Data->View;

            if (!Instances.empty())
            {
                MainPipeline->Begin(commandBuffer);

                RendererCommand::SetViewport(0.0f, 0.0f, s_Data->Width, s_Data->Height);

                Set->SetUniform("Camera", "u_ViewProjection", &vp);
                Set->Upload(commandBuffer);

                Set->SetUniform("u_Instances", "Instances", Instances.data(), Instances.size() * sizeof(InstanceData));
                Set->Upload(commandBuffer);

                Set->SetTexture("u_FontAtlasTextures", FontAtlasTextures.data(), (uint32_t)FontAtlasTextures.size());
                Set->Upload(commandBuffer);

                DrawElementsIndirectCommand command = {
                    6, (uint32_t)Instances.size(), 0, 0, 0
                };
                Buffer->SetData(&command, sizeof(command));

                commandBuffer->BindSets(&Set);
                RendererCommand::DrawIndirect(DrawType::TRIANGLE_STRIP, nullptr, Buffer);

                Engine::Get().GetStats().DrawCalls += 1;
                Engine::Get().GetStats().TrianglesCount += (uint32_t)Instances.size() * 2;

                MainPipeline->End(commandBuffer);

                if (Engine::Get().GetSettings().MousePicking)
                {
                    PipelineSpecification pspec = {};
                    pspec.pShader               = PickingShader;
                    pspec.TransparencyEnabled   = false;
                    pspec.ColorTargets[0]       = s_Data->MainPickingTexture;
                    pspec.DepthTarget           = s_Data->MainDepthTexture;
                    pspec.DepthWrite            = false;
                    pspec.ClearTargets          = false;
                    pspec.Samples               = 1;
                    pspec.DebugName             = "TextPickingPipeline";

                    auto pipeline               = Pipeline::Get(pspec);

                    pipeline->Begin(commandBuffer);

                    RendererCommand::SetViewport(0.0f, 0.0f, s_Data->Width, s_Data->Height);

                    PickingSet->SetUniform("Camera", "u_ViewProjection", &vp);
                    PickingSet->Upload(commandBuffer);

                    PickingSet->SetUniform("u_Instances", "Instances", Instances.data(), Instances.size() * sizeof(InstanceData));
                    PickingSet->Upload(commandBuffer);

                    size_t bufferSize = sizeof(int) + sizeof(int) * EntityBuffer.EntityIDS.size();
                    PickingSet->PrepareStorageBuffer("EntityBuffer", bufferSize);
                    PickingSet->SetStorage("EntityBuffer", "Count", &EntityBuffer.Count, sizeof(int));
                    PickingSet->SetStorage("EntityBuffer", "EnttIDs", EntityBuffer.EntityIDS.data(), sizeof(int) * EntityBuffer.EntityIDS.size());
                    PickingSet->Upload(commandBuffer);

                    commandBuffer->BindSets(&PickingSet);
                    RendererCommand::DrawIndirect(DrawType::TRIANGLE_STRIP, nullptr, Buffer);

                    Engine::Get().GetStats().DrawCalls += 1;

                    pipeline->End(commandBuffer);
                }
            }
        }

    } // Text

} // namespace KTN
