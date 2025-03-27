#include "ktnpch.h"
#include "Renderer.h"
#include "Koten/Graphics/Pipeline.h"
#include "Koten/Graphics/Shader.h"
#include "Koten/Graphics/DescriptorSet.h"
#include "Koten/Graphics/RendererCommand.h"



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

		struct RenderData
		{
			Ref<Texture2D> RenderTarget			= nullptr;
			Ref<Texture2D> ResolveTexture		= nullptr;
			Ref<Texture2D> MainTexture			= nullptr;
			Ref<Texture2D> MainPickingTexture	= nullptr;
			Ref<Texture2D> MainDepthTexture		= nullptr;
			uint32_t Width						= 0;
			uint32_t Height						= 0;
			glm::mat4 Projection				= { 1.0f };
			glm::mat4 View						= { 1.0f };
			uint8_t Samples						= 1;
			glm::vec4 ClearColor				= { 0.0f, 0.0f, 0.0f, 1.0f };

			Ref<Texture2D> WhiteTexture			= nullptr;

			Ref<Shader> FinalPassShader			= nullptr;
			Ref<DescriptorSet> FinalPassSet		= nullptr;
		};

		namespace R2D
		{
			struct QuadInstanceData
			{
				glm::mat4 Transform;
				glm::vec4 Color;
				glm::vec2 UVMin;
				glm::vec2 UVMax;
				alignas(16) float TexIndex;
			};

			struct EntityBufferData
			{
				int Count = 0;
				std::vector<int> EntityIDS;
			};

			struct QuadData
			{
				Ref<Shader> MainShader				= nullptr;
				Ref<DescriptorSet> Set				= nullptr;
				Ref<Pipeline> MainPipeline			= nullptr;
				Ref<IndirectBuffer> Buffer			= nullptr;

				Ref<VertexArray> VAO				= nullptr;

				std::vector<QuadInstanceData> Instances;

				uint32_t TextureSlotIndex = 1;
				std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;

				Ref<Shader> PickingShader = nullptr;
				Ref<DescriptorSet> PickingSet = nullptr;
				EntityBufferData EntityBuffer = {};

				void Init();
				void Begin();
				void StartBatch();
				void FlushAndReset();
				void Flush();
				void Submit(const RenderCommand& p_Command);
			};

		} // namespace R2D

	} // namespace

	static RenderData* s_Data					= nullptr;
	static R2D::QuadData* s_QuadData			= nullptr;

	void Renderer::Init()
	{
		KTN_PROFILE_FUNCTION();

		s_Data = new RenderData();

		uint32_t whiteTextureData = 0xffffffff;
		s_Data->WhiteTexture = Texture2D::Create({}, (uint8_t*)&whiteTextureData, sizeof(uint32_t));

		s_Data->FinalPassShader = Shader::Create("Assets/Shaders/FinalPass.glsl");
		s_Data->FinalPassSet = DescriptorSet::Create({ 0, s_Data->FinalPassShader });

		// R2D
		{
			s_QuadData = new R2D::QuadData();
			s_QuadData->Init();
		}
	}

	void Renderer::Shutdown()
	{
		KTN_PROFILE_FUNCTION();

		delete s_Data;
		delete s_QuadData;
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

		s_Data->RenderTarget		= p_Info.RenderTarget;
		s_Data->Width				= p_Info.Width;
		s_Data->Height				= p_Info.Height;
		s_Data->Projection			= p_Info.Projection;
		s_Data->View				= p_Info.View;
		s_Data->Samples				= p_Info.Samples;

		TextureSpecification tspec  = {};
		tspec.Format				= TextureFormat::RGBA32_FLOAT;
		tspec.Usage					= TextureUsage::TEXTURE_COLOR_ATTACHMENT;
		tspec.Width					= p_Info.Width;
		tspec.Height				= p_Info.Height;
		tspec.GenerateMips			= false;
		tspec.AnisotropyEnable		= false;
		tspec.Samples				= p_Info.Samples;
		tspec.DebugName				= "MainTexture";

		s_Data->MainTexture			= Texture2D::Get(tspec);

		tspec.Format				= TextureFormat::R32_INT;
		tspec.Samples				= 1;
		tspec.DebugName				= "MainPickingTexture";

		s_Data->MainPickingTexture	= Texture2D::Get(tspec);

		if (p_Info.Samples > 1)
		{
			tspec.Format			= TextureFormat::RGBA32_FLOAT;
			tspec.DebugName			= "MainResolveTexture";

			s_Data->ResolveTexture  = Texture2D::Get(tspec);
		}

		tspec.Samples				= p_Info.Samples;
		tspec.Format				= TextureFormat::D32_FLOAT;
		tspec.Usage					= TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT;
		tspec.DebugName				= "MainDepthTexture";

		s_Data->MainDepthTexture	= Texture2D::Get(tspec);

		if (p_Info.Clear)
			Clear();

		// R2D
		{
			s_QuadData->Begin();
		}
	}

	void Renderer::End()
	{
		KTN_PROFILE_FUNCTION();

		auto commandBuffer			= RendererCommand::GetCurrentCommandBuffer();

		// R2D
		{
			s_QuadData->Flush();
		}

		// Final Pass
		{
			PipelineSpecification pspec = {};
			pspec.ColorTargets[0]		= s_Data->RenderTarget;
			pspec.pShader				= s_Data->FinalPassShader;
			pspec.SwapchainTarget		= s_Data->RenderTarget == nullptr;
			pspec.ClearTargets			= true;
			pspec.DepthTest				= false;
			pspec.DepthWrite			= false;
			pspec.DebugName				= "FinalPassPipeline";

			auto pipeline				= Pipeline::Get(pspec);

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

		s_QuadData->Submit(p_Command);
	}

	Ref<Texture2D> Renderer::GetPickingTexture()
	{
		return s_Data->MainPickingTexture;
	}

	namespace
	{
		namespace R2D
		{
			void QuadData::Init()
			{
				MainShader = Shader::Create("Assets/Shaders/R2D_Quad.glsl");
				Set = DescriptorSet::Create({ 0, MainShader });

				VAO = VertexArray::Create();

				float vertices[] = {
					// positions
					-0.5f, -0.5f, 0.0f,
					 0.5f, -0.5f, 0.0f,
					 0.5f,  0.5f, 0.0f,
					-0.5f,  0.5f, 0.0f
				};

				auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
				vbo->SetLayout({
					{ DataType::Float3 , "a_Position"	}
				});
				VAO->SetVertexBuffer(vbo);

				uint32_t indices[] = {
					0, 1, 3, // first triangle
					1, 2, 3  // second triangle
				};
				auto ebo = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
				VAO->SetIndexBuffer(ebo);

				TextureSlots.fill(s_Data->WhiteTexture);

				Buffer = IndirectBuffer::Create(sizeof(DrawElementsIndirectCommand));

				if (Engine::GetSettings().MousePicking)
				{
					PickingShader = Shader::Create("Assets/Shaders/R2D_Picking.glsl");
					PickingSet = DescriptorSet::Create({ 0, PickingShader });
				}
			}

			void QuadData::Begin()
			{
				PipelineSpecification pspec = {};
				pspec.pShader				= MainShader;
				pspec.TransparencyEnabled	= false;
				pspec.ColorTargets[0]		= s_Data->MainTexture;
				pspec.DepthTarget			= s_Data->MainDepthTexture;
				pspec.ClearTargets			= false;
				pspec.ResolveTexture		= s_Data->ResolveTexture;
				pspec.Samples				= s_Data->Samples;
				pspec.DebugName				= "QuadMainPipeline";

				MainPipeline = Pipeline::Get(pspec);

				StartBatch();
			}

			void QuadData::StartBatch()
			{
				Instances.clear();
				TextureSlotIndex = 1;

				EntityBuffer.Count = 0;
				EntityBuffer.EntityIDS.clear();
			}

			void QuadData::FlushAndReset()
			{
				Flush();
				StartBatch();
			}

			void QuadData::Flush()
			{
				auto commandBuffer = RendererCommand::GetCurrentCommandBuffer();
				auto vp = s_Data->Projection * s_Data->View;

				if (!Instances.empty())
				{
					MainPipeline->Begin(commandBuffer);

					RendererCommand::SetViewport(0.0f, 0.0f, s_Data->Width, s_Data->Height);

					Set->SetUniform("Camera", "u_ViewProjection", &vp);
					Set->Upload(commandBuffer);

					Set->SetUniform("u_Instances", "Instances", Instances.data(), Instances.size() * sizeof(QuadInstanceData));
					Set->Upload(commandBuffer);

					Set->SetTexture("u_Textures", TextureSlots.data(), (uint32_t)TextureSlots.size());
					Set->Upload(commandBuffer);

					DrawElementsIndirectCommand command = {
						6, (uint32_t)Instances.size(), 0, 0, 0
					};
					Buffer->SetData(&command, sizeof(command));

					commandBuffer->BindSets(&Set);
					RendererCommand::DrawIndexedIndirect(DrawType::TRIANGLES, VAO, Buffer);

					Engine::GetStats().DrawCalls += 1;
					Engine::GetStats().TrianglesCount += (uint32_t)Instances.size() * 2;

					MainPipeline->End(commandBuffer);

					if (Engine::GetSettings().MousePicking)
					{
						PipelineSpecification pspec = {};
						pspec.pShader = PickingShader;
						pspec.TransparencyEnabled = false;
						pspec.ColorTargets[0] = s_Data->MainPickingTexture;
						pspec.DepthTarget = s_Data->MainDepthTexture;
						pspec.DepthWrite = false;
						pspec.ClearTargets = false;
						pspec.Samples = 1;
						pspec.DebugName = "QuadPickingPipeline";

						auto pipeline = Pipeline::Get(pspec);

						pipeline->Begin(commandBuffer);

						RendererCommand::SetViewport(0.0f, 0.0f, s_Data->Width, s_Data->Height);

						PickingSet->SetUniform("Camera", "u_ViewProjection", &vp);
						PickingSet->Upload(commandBuffer);

						PickingSet->SetUniform("u_Instances", "Instances", Instances.data(), Instances.size() * sizeof(QuadInstanceData));
						PickingSet->Upload(commandBuffer);

						size_t bufferSize = sizeof(int) + sizeof(int) * EntityBuffer.EntityIDS.size();
						PickingSet->PrepareStorageBuffer("EntityBuffer", bufferSize);
						PickingSet->SetStorage("EntityBuffer", "Count", &EntityBuffer.Count, sizeof(int));
						PickingSet->SetStorage("EntityBuffer", "EnttIDs", EntityBuffer.EntityIDS.data(), sizeof(int) * EntityBuffer.EntityIDS.size());
						PickingSet->Upload(commandBuffer);

						commandBuffer->BindSets(&PickingSet);
						RendererCommand::DrawIndexedIndirect(DrawType::TRIANGLES, VAO, Buffer);

						Engine::GetStats().DrawCalls += 1;

						pipeline->End(commandBuffer);
					}
				}
			}
			void QuadData::Submit(const RenderCommand& p_Command)
			{
				if (Instances.size() >= (size_t)MaxInstances)
					FlushAndReset();

				EntityBuffer.EntityIDS.push_back(p_Command.EntityID);
				EntityBuffer.Count++;

				float textureIndex = 0.0f; // White texture
				if (p_Command.SpriteData.Texture)
				{
					for (uint32_t i = 1; i < TextureSlotIndex; i++)
					{
						// TODO: compare texture UUIDs
						if (*TextureSlots[i].get() == *p_Command.SpriteData.Texture.get())
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
						TextureSlots[TextureSlotIndex] = p_Command.SpriteData.Texture;
						TextureSlotIndex++;
					}
				}

				QuadInstanceData data = {};
				data.Transform = p_Command.Transform;
				data.Color = p_Command.SpriteData.Color;
				data.TexIndex = textureIndex;
				data.UVMin = { 0.0f, 0.0f };
				data.UVMax = { 1.0f, 1.0f };

				if (p_Command.SpriteData.Texture)
				{
					auto scale = p_Command.SpriteData.Scale;
					auto offset = p_Command.SpriteData.Offset;
					glm::vec2 size;
					if (p_Command.SpriteData.Size.x == 0.0f && p_Command.SpriteData.Size.y == 0.0f)
					{
						size = { p_Command.SpriteData.Texture->GetWidth(), p_Command.SpriteData.Texture->GetHeight() };
					}
					else
						size = p_Command.SpriteData.Size;

					float customTileSizeX = (scale.x == 0.0f) ? 1.0f : scale.x;
					float customTileSizeY = (scale.y == 0.0f) ? 1.0f : scale.y;

					glm::vec2 min = {
						(p_Command.SpriteData.BySize ? offset.x * size.x : offset.x) / p_Command.SpriteData.Texture->GetWidth(),
						(p_Command.SpriteData.BySize ? offset.y * size.y : offset.y) / p_Command.SpriteData.Texture->GetHeight()
					};

					glm::vec2 max = {
						(p_Command.SpriteData.BySize ? (offset.x + customTileSizeX) * size.x : offset.x + (customTileSizeX * size.x)) / p_Command.SpriteData.Texture->GetWidth(),
						(p_Command.SpriteData.BySize ? (offset.y + customTileSizeY) * size.y : offset.y + (customTileSizeY * size.x)) / p_Command.SpriteData.Texture->GetHeight()
					};

					data.UVMin = min;
					data.UVMax = max;
				}

				Instances.push_back(data);
			}
		} // namespace R2D

	} // namespace

} // namespace KTN
