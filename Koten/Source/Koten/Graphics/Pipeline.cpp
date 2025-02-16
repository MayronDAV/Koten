#include "ktnpch.h"
#include "Koten/Core/Application.h"
#include "Pipeline.h"

#include "Platform/OpenGL/GLPipeline.h"



namespace KTN
{
	struct PipelineAsset
	{
		Ref<Pipeline> pPipeline;
		double TimeSinceLastAccessed;
	};
	static std::unordered_map<uint64_t, PipelineAsset> s_PipelineCache;
	static const double s_CacheLifeTime = 0.1;

	uint32_t Pipeline::GetWidth()
	{
		KTN_PROFILE_FUNCTION();

		if (m_Spec.SwapchainTarget)
		{
			if (Engine::GetAPI() == RenderAPI::OpenGL)
				return Application::Get().GetWindow()->GetWidth();
		}

		if (m_Spec.ResolveTexture)
		{
			return m_Spec.ResolveTexture->GetWidth();
		}

		if (m_Spec.ColorTargets[0])
		{
			return m_Spec.ColorTargets[0]->GetWidth();
		}

		if (m_Spec.DepthTarget)
			return m_Spec.DepthTarget->GetWidth();


		KTN_CORE_ERROR("Invalid pipeline width");
		return 0;
	}

	uint32_t Pipeline::GetHeight()
	{
		KTN_PROFILE_FUNCTION();

		if (m_Spec.SwapchainTarget)
		{
			if (Engine::GetAPI() == RenderAPI::OpenGL)
				return Application::Get().GetWindow()->GetHeight();
		}

		if (m_Spec.ResolveTexture)
		{
			return m_Spec.ResolveTexture->GetHeight();
		}

		if (m_Spec.ColorTargets[0])
		{
			return m_Spec.ColorTargets[0]->GetHeight();
		}

		if (m_Spec.DepthTarget)
			return m_Spec.DepthTarget->GetHeight();

		KTN_CORE_ERROR("Invalid pipeline height");
		return 0;
	}

	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLPipeline>(p_Spec);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<Pipeline> Pipeline::Get(const PipelineSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(p_Spec.pShader != nullptr);

		uint64_t hash = 0;

		for (int i = 0; i < MAX_RENDER_TARGETS; i++)
		{
			HashCombine(hash, p_Spec.ColorTargets[i].get(), p_Spec.BlendModes[i]);
		}

		HashCombine(hash, p_Spec.pShader.get(), p_Spec.ResolveTexture.get(), p_Spec.DepthTarget.get());

		HashCombine(hash, p_Spec.pCullMode, p_Spec.pFrontFace, p_Spec.pDrawType, p_Spec.LineWidth,
			p_Spec.pPolygonMode);

		HashCombine(hash, p_Spec.TransparencyEnabled, p_Spec.ClearTargets, p_Spec.DepthTest, p_Spec.DepthWrite,
			p_Spec.SwapchainTarget, p_Spec.BuildMipFramebuffers);

		HashCombine(hash, p_Spec.DepthBiasEnabled, p_Spec.ConstantFactor, p_Spec.SlopeFactor);

		HashCombine(hash, p_Spec.StencilTest, p_Spec.pStencilFace, p_Spec.pStencilCompare,
			p_Spec.StencilReference, p_Spec.StencilCompareMask, p_Spec.StencilWriteMask,
			p_Spec.FailOp, p_Spec.DepthFailOp, p_Spec.PassOp);

		HashCombine(hash, p_Spec.Samples, p_Spec.DebugName);

		auto found = s_PipelineCache.find(hash);
		if (found != s_PipelineCache.end() && found->second.pPipeline)
		{
			found->second.TimeSinceLastAccessed = Time::GetTime();
			return found->second.pPipeline;
		}

		Ref<Pipeline> pipeline = Create(p_Spec);
		s_PipelineCache[hash] = { pipeline, Time::GetTime() };
		return pipeline;
	}

	void Pipeline::ClearCache()
	{
		KTN_PROFILE_FUNCTION();

		s_PipelineCache.clear();
	}

	void Pipeline::DeleteUnusedCache()
	{
		KTN_PROFILE_FUNCTION();

		static std::size_t keysToDelete[256];
		std::size_t keysToDeleteCount = 0;

		for (auto&& [key, value] : s_PipelineCache)
		{
			if (value.pPipeline && (Time::GetTime() - value.TimeSinceLastAccessed) > s_CacheLifeTime)
			{
				keysToDelete[keysToDeleteCount] = key;
				keysToDeleteCount++;
			}

			if (keysToDeleteCount >= 256)
				break;
		}

		for (std::size_t i = 0; i < keysToDeleteCount; i++)
		{
			s_PipelineCache[keysToDelete[i]].pPipeline = nullptr;
			s_PipelineCache.erase(keysToDelete[i]);
		}
	}


} // namespace KTN
