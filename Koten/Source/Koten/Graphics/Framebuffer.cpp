#include "ktnpch.h"
#include "Framebuffer.h"

#include "Platform/OpenGL/GLFramebuffer.h"



namespace KTN
{
	struct FramebufferAsset
	{
		Ref<Framebuffer> FrameBuffer;
		float TimeSinceLastAccessed;
	};
	static std::unordered_map<uint64_t, FramebufferAsset> s_FramebufferCache;
	static const float s_CacheLifeTime = 1.0f;

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& p_Spec)
	{
		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLFramebuffer>(p_Spec);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<Framebuffer> Framebuffer::Get(const FramebufferSpecification& p_Spec)
	{
		KTN_CORE_ASSERT(p_Spec.RenderPass);
		KTN_CORE_ASSERT(p_Spec.Attachments);
		KTN_CORE_ASSERT(p_Spec.AttachmentCount > 0);

		uint64_t hash = 0;
		HashCombine(hash, p_Spec.Width, p_Spec.Height, p_Spec.DebugName, p_Spec.MipIndex, p_Spec.Samples, p_Spec.AttachmentCount);

		for (uint32_t i = 0; i < p_Spec.AttachmentCount; i++)
		{
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
			found->second.TimeSinceLastAccessed = (float)Time::GetTime();
			return found->second.FrameBuffer;
		}

		auto framebuffer = Create(p_Spec);
		s_FramebufferCache[hash] = { framebuffer, (float)Time::GetTime() };
		return framebuffer;
	}

	void Framebuffer::ClearCache()
	{
		s_FramebufferCache.clear();
	}

	void Framebuffer::DeleteUnusedCache()
	{
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