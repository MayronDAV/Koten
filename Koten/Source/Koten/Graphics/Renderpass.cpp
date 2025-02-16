#include "ktnpch.h"
#include "Renderpass.h"

#include "Platform/OpenGL/GLRenderpass.h"



namespace KTN
{
	struct RenderpassAsset
	{
		Ref<Renderpass> RenderPass;
		double TimeSinceLastAccessed;
	};
	static std::unordered_map<uint64_t, RenderpassAsset> s_RenderPassCache;
	static const double s_CacheLifeTime = 0.1;

	Ref<Renderpass> Renderpass::Create(const RenderpassSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLRenderpass>(p_Spec);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<Renderpass> Renderpass::Get(const RenderpassSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(p_Spec.Attachments);
		KTN_CORE_ASSERT(p_Spec.AttachmentCount > 0);

		uint64_t hash = 0;
		HashCombine(hash, p_Spec.Samples, p_Spec.Clear, p_Spec.SwapchainTarget, p_Spec.DebugName, p_Spec.AttachmentCount);

		for (uint32_t i = 0; i < p_Spec.AttachmentCount; i++)
		{
			auto& texture = p_Spec.Attachments[i];
			if (texture)
			{
				HashCombine(hash, texture.get());
			}
		}

		if (p_Spec.ResolveTexture)
		{
			HashCombine(hash, p_Spec.ResolveTexture.get());
		}

		auto found = s_RenderPassCache.find(hash);
		if (found != s_RenderPassCache.end() && found->second.RenderPass)
		{
			found->second.TimeSinceLastAccessed = Time::GetTime();
			return found->second.RenderPass;
		}

		auto renderPass = Create(p_Spec);
		s_RenderPassCache[hash] = { renderPass, Time::GetTime() };
		return renderPass;
	}

	void Renderpass::ClearCache()
	{
		KTN_PROFILE_FUNCTION();

		s_RenderPassCache.clear();
	}

	void Renderpass::DeleteUnusedCache()
	{
		KTN_PROFILE_FUNCTION();

		static std::size_t keysToDelete[256];
		std::size_t keysToDeleteCount = 0;

		for (auto&& [key, value] : s_RenderPassCache)
		{
			if (value.RenderPass && (Time::GetTime() - value.TimeSinceLastAccessed) > s_CacheLifeTime)
			{
				keysToDelete[keysToDeleteCount] = key;
				keysToDeleteCount++;
			}

			if (keysToDeleteCount >= 256)
				break;
		}

		for (std::size_t i = 0; i < keysToDeleteCount; i++)
		{
			s_RenderPassCache[keysToDelete[i]].RenderPass = nullptr;
			s_RenderPassCache.erase(keysToDelete[i]);
		}
	}

} // namespace KTN
