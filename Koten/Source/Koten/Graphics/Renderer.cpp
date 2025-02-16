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
		struct RenderData
		{
			Ref<Texture2D> RenderTarget			= nullptr;
			Ref<Texture2D> ResolveTexture		= nullptr;
			Ref<Texture2D> MainTexture			= nullptr;
			Ref<Texture2D> MainDepthTexture		= nullptr;
			uint32_t Width						= 0;
			uint32_t Height						= 0;
			glm::mat4 Projection				= { 1.0f };
			glm::mat4 View						= { 1.0f };

			Ref<Texture2D> WhiteTexture			= nullptr;

			Ref<Shader> FinalPassShader			= nullptr;
			Ref<DescriptorSet> FinalPassSet		= nullptr;
		};

		struct QuadData
		{
			Ref<Shader> MainShader				= nullptr;
			Ref<DescriptorSet> Set				= nullptr;
			Ref<Pipeline> MainPipeline			= nullptr;

			Ref<VertexArray> VAO				= nullptr;
		};

	} // namespace

	static RenderData* s_Data					= nullptr;
	static QuadData* s_QuadData					= nullptr;

	void Renderer::Init()
	{
		KTN_PROFILE_FUNCTION();

		s_Data = new RenderData();
		s_QuadData = new QuadData();

		s_QuadData->MainShader = Shader::Create("Assets/Shaders/ShaderTest.glsl");
		s_QuadData->Set = DescriptorSet::Create({ 0, s_QuadData->MainShader });

		s_QuadData->VAO = VertexArray::Create();

		float vertices[] = {
			// positions      // texcoords  // colors
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f
		};

		auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
		vbo->SetLayout({
			{ DataType::Float3 , "a_Position"	},
			{ DataType::Float2 , "a_Texcoord"	},
			{ DataType::Float3 , "a_Color"		},
			});
		s_QuadData->VAO->SetVertexBuffer(vbo);

		uint32_t indices[] = {
			0, 1, 3, // first triangle
			1, 2, 3  // second triangle
		};
		auto ebo = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		s_QuadData->VAO->SetIndexBuffer(ebo);

		uint32_t whiteTextureData = 0xffffffff;
		s_Data->WhiteTexture = Texture2D::Create({}, (uint8_t*)&whiteTextureData, sizeof(uint32_t));

		s_Data->FinalPassShader = Shader::Create("Assets/Shaders/FinalPass.glsl");
		s_Data->FinalPassSet = DescriptorSet::Create({ 0, s_Data->FinalPassShader });
	}

	void Renderer::Shutdown()
	{
		KTN_PROFILE_FUNCTION();

		delete s_Data;
		delete s_QuadData;
	}

	void Renderer::Begin(const RenderBeginInfo& p_Info)
	{
		KTN_PROFILE_FUNCTION();

		s_Data->RenderTarget		= p_Info.RenderTarget;
		s_Data->Width				= p_Info.Width;
		s_Data->Height				= p_Info.Height;
		s_Data->Projection			= p_Info.pCamera.GetProjection();
		s_Data->View				= p_Info.View;

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

		if (p_Info.Samples > 1)
		{
			tspec.Samples			= 1;
			tspec.DebugName			= "MainResolveTexture";

			s_Data->ResolveTexture  = Texture2D::Get(tspec);
		}

		tspec.Samples				= p_Info.Samples;
		tspec.Format				= TextureFormat::D32_FLOAT;
		tspec.Usage					= TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT;
		tspec.DebugName				= "MainDepthTexture";

		s_Data->MainDepthTexture	= Texture2D::Get(tspec);


		PipelineSpecification pspec = {};
		pspec.pShader				= s_QuadData->MainShader;
		pspec.TransparencyEnabled	= false;
		pspec.ColorTargets[0]		= s_Data->MainTexture;
		pspec.DepthTarget			= s_Data->MainDepthTexture;
		pspec.ClearTargets			= true;
		pspec.ClearColor			= { 0.0f, 0.0f, 0.0f, 1.0f };
		pspec.ResolveTexture		= s_Data->ResolveTexture;
		pspec.Samples				= p_Info.Samples;
		pspec.DebugName				= "MainPipeline";

		s_QuadData->MainPipeline	= Pipeline::Get(pspec);

		auto commandBuffer = RendererCommand::GetCurrentCommandBuffer();

		s_QuadData->MainPipeline->Begin(commandBuffer);

		commandBuffer->SetViewport(0.0f, 0.0f, p_Info.Width, p_Info.Height);
	}

	void Renderer::End()
	{
		KTN_PROFILE_FUNCTION();

		auto commandBuffer = RendererCommand::GetCurrentCommandBuffer();
		
		s_QuadData->MainPipeline->End(commandBuffer);

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

		auto commandBuffer = RendererCommand::GetCurrentCommandBuffer();

		s_QuadData->Set->SetTexture("u_Texture", p_Command.SpriteData.Texture ? p_Command.SpriteData.Texture : s_Data->WhiteTexture);
		s_QuadData->Set->Upload(commandBuffer);

		commandBuffer->BindSets(&s_QuadData->Set);

		glm::mat4 matrix = s_Data->Projection * s_Data->View * p_Command.Transform;
		s_QuadData->MainShader->SetPushValue("Matrix", &matrix);
		s_QuadData->MainShader->BindPushConstants(commandBuffer);

		RendererCommand::DrawIndexed(DrawType::TRIANGLES, s_QuadData->VAO);

		Engine::GetStats().DrawCalls		+= 1;
		Engine::GetStats().TrianglesCount	+= 2;
	}

} // namespace KTN
