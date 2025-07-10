#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Definitions.h"
#include "Koten/Asset/Asset.h"



namespace KTN
{
	class KTN_API Texture : public Asset
	{
		public:
			virtual ~Texture() = default;

			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			virtual uint32_t GetChannels() const = 0;
			virtual uint32_t GetBytesPerChannels() const = 0;
			virtual uint32_t GetMipLevels() const = 0;
			virtual int GetSampleCount() const = 0;

			virtual uint64_t GetEstimatedSize() const = 0;

			virtual const TextureSpecification& GetSpecification() const = 0;

			virtual void GenerateMipmap(CommandBuffer* p_CommandBuffer) = 0;

			virtual void SetData(const void* p_Data, size_t p_Size) = 0;

			virtual void Bind(uint32_t p_Slot = 0) = 0;
			virtual void Unbind() = 0;

			virtual std::vector<uint8_t> GetData() const = 0;

			virtual bool operator== (const Texture& p_Other) const = 0;

			static bool IsDepthStencilFormat(TextureFormat p_Format)
			{
				return p_Format == TextureFormat::D24_S8_UINT || p_Format == TextureFormat::D16_S8_UINT ||
					p_Format == TextureFormat::D32_FLOAT_S8_UINT;
			}

			static bool IsDepthFormat(TextureFormat p_Format)
			{
				return p_Format == TextureFormat::D16 || p_Format == TextureFormat::D32_FLOAT ||
					p_Format == TextureFormat::D24_S8_UINT || p_Format == TextureFormat::D16_S8_UINT ||
					p_Format == TextureFormat::D32_FLOAT_S8_UINT;
			}

			static bool IsStencilFormat(TextureFormat p_Format)
			{
				return p_Format == TextureFormat::D24_S8_UINT || p_Format == TextureFormat::D16_S8_UINT ||
					p_Format == TextureFormat::D32_FLOAT_S8_UINT;
			}

			bool IsDepthStencil() const { return IsDepthStencilFormat(GetSpecification().Format); }
			bool IsDepth() const { return IsDepthFormat(GetSpecification().Format); }
			bool IsStencil() const { return IsStencilFormat(GetSpecification().Format); }

			bool IsSampled() const { return GetSpecification().Usage == TextureUsage::TEXTURE_SAMPLED; }
			bool IsColorAttachment() const { return GetSpecification().Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT; }
			bool IsDepthStencilAttachment() const { return GetSpecification().Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT; }
			bool IsStorage() const { return GetSpecification().Usage == TextureUsage::TEXTURE_STORAGE; }

			static void ClearCache();
			static void DeleteUnusedCache();
	};


	class KTN_API Texture2D : public Texture
	{
		public:
			virtual void Resize(uint32_t p_Width, uint32_t p_Height) = 0;

			static Ref<Texture2D> Create(const TextureSpecification& p_Spec = {});
			static Ref<Texture2D> Create(const TextureSpecification& p_Spec, const uint8_t* p_Data, size_t p_Size);

			static Ref<Texture2D> Get(const TextureSpecification& p_Spec = {});

			ASSET_CLASS_METHODS(Texture2D)
	};

} // namespace KTN
