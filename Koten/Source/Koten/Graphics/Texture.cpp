#include "ktnpch.h"
#include "Texture.h"

#include "Platform/OpenGL/GLTexture.h"



namespace KTN
{
	struct TextureAsset
	{
		Ref<Texture> pTexture = nullptr;
		double TimeSinceLastAccessed = 0;
	};
	static std::unordered_map<uint64_t, TextureAsset> s_TextureCache;
	static const double s_CacheLifeTime = 1.0;

	void Texture::ClearCache()
	{
		s_TextureCache.clear();
	}

	void Texture::DeleteUnusedCache()
	{
		static std::size_t keysToDelete[256];
		std::size_t keysToDeleteCount = 0;

		for (auto&& [key, value] : s_TextureCache)
		{
			if (value.pTexture && (Time::GetTime() - value.TimeSinceLastAccessed) > s_CacheLifeTime)
			{
				keysToDelete[keysToDeleteCount] = key;
				keysToDeleteCount++;
			}

			if (keysToDeleteCount >= 256)
				break;
		}

		for (std::size_t i = 0; i < keysToDeleteCount; i++)
		{
			s_TextureCache[keysToDelete[i]].pTexture = nullptr;
			s_TextureCache.erase(keysToDelete[i]);
		}
	}

	Ref<Texture2D> Texture2D::Create(const TextureSpecification& p_Spec)
	{
		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLTexture2D>(p_Spec);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const TextureSpecification& p_Spec, const uint8_t* p_Data, size_t p_Size)
	{
		if (Engine::GetAPI() == RenderAPI::OpenGL)
			return CreateRef<GLTexture2D>(p_Spec, p_Data, p_Size);

		KTN_CORE_ERROR("Unsupported API!");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Get(const TextureSpecification& p_Spec)
	{
		uint64_t hash = 0;

		HashCombine(hash, glm::value_ptr(p_Spec.BorderColor), p_Spec.Samples);

		HashCombine(hash, p_Spec.Usage, p_Spec.Format, p_Spec.Access , p_Spec.WrapU, p_Spec.WrapV, p_Spec.WrapW, p_Spec.MinFilter, p_Spec.MagFilter);

		HashCombine(hash, p_Spec.Height, p_Spec.Width, p_Spec.AnisotropyEnable, p_Spec.GenerateMips, p_Spec.SRGB);

		HashCombine(hash, p_Spec.DebugName);

		auto found = s_TextureCache.find(hash);
		if (found != s_TextureCache.end() && found->second.pTexture)
		{
			found->second.TimeSinceLastAccessed = Time::GetTime();
			return As<Texture, Texture2D>(found->second.pTexture);
		}

		auto texture = Create(p_Spec);
		s_TextureCache[hash] = { texture, Time::GetTime() };
		return texture;
	}

} // namespace KTN
