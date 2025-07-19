#include "ktnpch.h"
#include "RendererCommand.h"



namespace KTN
{
	RendererAPI* RendererCommand::s_API = nullptr;

	void RendererCommand::Init()
	{
		KTN_PROFILE_FUNCTION();

		s_API = RendererAPI::Create();
	}

	void RendererCommand::Release()
	{
		KTN_PROFILE_FUNCTION();

		if (s_API)
			delete s_API;
	}

	void RendererCommand::Begin()
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(s_API);
		s_API->Begin();
	}

	void RendererCommand::End()
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(s_API);
		s_API->End();
	}

	void RendererCommand::OnResize(uint32_t p_Width, uint32_t p_Height)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(s_API);
		s_API->OnResize(p_Width, p_Height);
	}

	void RendererCommand::ClearColor(const glm::vec4& p_Color)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(s_API);
		s_API->ClearColor(p_Color);
	}

	void RendererCommand::ClearRenderTarget(const Ref<Texture2D>& p_Texture, uint32_t p_Value)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(s_API);

		s_API->ClearRenderTarget(p_Texture, p_Value);
	}

	void RendererCommand::ClearRenderTarget(const Ref<Texture2D>& p_Texture, const glm::vec4& p_Value)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(s_API);

		s_API->ClearRenderTarget(p_Texture, p_Value);
	}

	void* RendererCommand::ReadPixel(const Ref<Texture2D>& p_Texture, uint32_t p_X, uint32_t p_Y)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(s_API);
		if (Engine::Get().GetSettings().MousePicking)
			return s_API->ReadPixel(p_Texture, p_X, p_Y);

		return nullptr;
	}

	void RendererCommand::SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height)
	{
		KTN_PROFILE_FUNCTION();

		GetCurrentCommandBuffer()->SetViewport(p_X, p_Y, p_Width, p_Height);
	}

	void RendererCommand::DispatchCompute(uint32_t p_NumGroups_X, uint32_t p_NumGroups_Y, uint32_t p_NumGroups_Z)
	{
		KTN_PROFILE_FUNCTION();

		GetCurrentCommandBuffer()->DispatchCompute(p_NumGroups_X, p_NumGroups_Y, p_NumGroups_Z);
	}

	void RendererCommand::Draw(DrawType p_Type, const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount)
	{
		KTN_PROFILE_FUNCTION();

		GetCurrentCommandBuffer()->Draw(p_Type, p_VertexArray, p_VertexCount);
	}

	void RendererCommand::DrawIndirect(DrawType p_Type, const Ref<VertexArray>& p_VertexArray, const Ref<IndirectBuffer>& p_Buffer)
	{
		KTN_PROFILE_FUNCTION();

		GetCurrentCommandBuffer()->DrawIndirect(p_Type, p_VertexArray, p_Buffer);
	}

	void RendererCommand::DrawIndexed(DrawType p_Type, const Ref<VertexArray>& p_VertexArray)
	{
		KTN_PROFILE_FUNCTION();

		GetCurrentCommandBuffer()->DrawIndexed(p_Type, p_VertexArray);
	}

	void RendererCommand::DrawIndexedIndirect(DrawType p_Type, const Ref<VertexArray>& p_VertexArray, const Ref<IndirectBuffer>& p_Buffer)
	{
		KTN_PROFILE_FUNCTION();

		GetCurrentCommandBuffer()->DrawIndexedIndirect(p_Type, p_VertexArray, p_Buffer);
	}

	const Capabilities& RendererCommand::GetCapabilities()
	{
		KTN_CORE_ASSERT(s_API);
		return s_API->GetCapabilities();
	}

	CommandBuffer* RendererCommand::GetCurrentCommandBuffer()
	{
		KTN_CORE_ASSERT(s_API);
		return s_API->GetCurrentCommandBuffer();
	}

} // namespace KTN