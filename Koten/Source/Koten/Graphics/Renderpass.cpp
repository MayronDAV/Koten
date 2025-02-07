#include "ktnpch.h"
#include "Renderpass.h"

#include "Platform/OpenGL/GLRenderpass.h"



namespace KTN
{
	struct RenderpassAsset
	{
		Ref<Renderpass> RenderPass;
		float TimeSinceLastAccessed;
	};
	static std::unordered_map<uint64_t, RenderpassAsset> s_RenderPassCache;
	static const float s_CacheLifeTime = 0.1f;

	Ref<Renderpass> Renderpass::Create(const RenderpassSpecification& p_Spec)
	{
		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLRenderpass>(p_Spec);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<Renderpass> Renderpass::Get(const RenderpassSpecification& p_Spec)
	{
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
			found->second.TimeSinceLastAccessed = (float)Time::GetTime();
			return found->second.RenderPass;
		}

		auto renderPass = Create(p_Spec);
		s_RenderPassCache[hash] = { renderPass, (float)Time::GetTime() };
		return renderPass;
	}

	void Renderpass::ClearCache()
	{
		s_RenderPassCache.clear();
	}

	void Renderpass::DeleteUnusedCache()
	{
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
