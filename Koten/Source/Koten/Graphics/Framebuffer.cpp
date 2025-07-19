#include "ktnpch.h"
#include "Framebuffer.h"

#include "Platform/OpenGL/GLFramebuffer.h"



namespace KTN
{
	struct FramebufferAsset
	{
		Ref<Framebuffer> FrameBuffer;
		double TimeSinceLastAccessed;
	};
	static std::unordered_map<uint64_t, FramebufferAsset> s_FramebufferCache;
	static const double s_CacheLifeTime = 1.0f;

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION();

		if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLFramebuffer>(p_Spec);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<Framebuffer> Framebuffer::Get(const FramebufferSpecification& p_Spec)
	{
		KTN_PROFILE_FUNCTION();

		KTN_CORE_ASSERT(p_Spec.RenderPass);

		uint64_t hash = 0;
		HashCombine(hash, p_Spec.Width, p_Spec.Height, p_Spec.DebugName, p_Spec.MipIndex, p_Spec.Samples, p_Spec.AttachmentCount);

		for (uint32_t i = 0; i < p_Spec.AttachmentCount; i++)
		{
			KTN_CORE_ASSERT(p_Spec.Attachments);
			auto& texture = p_Spec.Attachments[i];
			if (texture)
			{
				HashCombine(hash, texture.get());
			}
		}

		HashCombine(hash, p_Spec.RenderPass.get());

		auto found = s_FramebufferCache.find(hash);
		if (found != s_FramebufferCache.end() && found->second.FrameBuffer)
		{
			found->second.TimeSinceLastAccessed = Time::GetTime();
			return found->second.FrameBuffer;
		}

		auto framebuffer = Create(p_Spec);
		s_FramebufferCache[hash] = { framebuffer, Time::GetTime() };
		return framebuffer;
	}

	void Framebuffer::ClearCache()
	{
		KTN_PROFILE_FUNCTION();

		s_FramebufferCache.clear();
	}

	void Framebuffer::DeleteUnusedCache()
	{
		KTN_PROFILE_FUNCTION();

		static std::size_t keysToDelete[256];
		std::size_t keysToDeleteCount = 0;

		for (auto&& [key, value] : s_FramebufferCache)
		{
			if (value.FrameBuffer && (Time::GetTime() - value.TimeSinceLastAccessed) > s_CacheLifeTime)
			{
				keysToDelete[keysToDeleteCount] = key;
				keysToDeleteCount++;
			}

			if (keysToDeleteCount >= 256)
				break;
		}

		for (std::size_t i = 0; i < keysToDeleteCount; i++)
		{
			s_FramebufferCache[keysToDelete[i]].FrameBuffer = nullptr;
			s_FramebufferCache.erase(keysToDelete[i]);
		}
	}

} // namespace KTN