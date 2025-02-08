#include "ktnpch.h"
#include "RendererCommand.h"



namespace KTN
{
	RendererAPI* RendererCommand::s_API = nullptr;

	void RendererCommand::Init()
	{
		s_API = RendererAPI::Create();
	}

	void RendererCommand::Release()
	{
		if (s_API)
			delete s_API;
	}

	void RendererCommand::Begin()
	{
		KTN_CORE_ASSERT(s_API);
		s_API->Begin();
	}

	void RendererCommand::End()
	{
		KTN_CORE_ASSERT(s_API);
		s_API->End();
	}

	void RendererCommand::OnResize(uint32_t p_Width, uint32_t p_Height)
	{
		KTN_CORE_ASSERT(s_API);
		s_API->OnResize(p_Width, p_Height);
	}

	void RendererCommand::ClearColor(const glm::vec4& p_Color)
	{
		KTN_CORE_ASSERT(s_API);
		s_API->ClearColor(p_Color);
	}

	void RendererCommand::SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height)
	{
		GetCurrentCommandBuffer()->SetViewport(p_X, p_Y, p_Width, p_Height);
	}

	void RendererCommand::DispatchCompute(uint32_t p_NumGroups_X, uint32_t p_NumGroups_Y, uint32_t p_NumGroups_Z)
	{
		GetCurrentCommandBuffer()->DispatchCompute(p_NumGroups_X, p_NumGroups_Y, p_NumGroups_Z);
	}

	void RendererCommand::Draw(DrawType p_Type, const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount)
	{
		GetCurrentCommandBuffer()->Draw(p_Type, p_VertexArray, p_VertexCount);
	}

	void RendererCommand::DrawIndexed(DrawType p_Type, const Ref<VertexArray>& p_VertexArray)
	{
		GetCurrentCommandBuffer()->DrawIndexed(p_Type, p_VertexArray);
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