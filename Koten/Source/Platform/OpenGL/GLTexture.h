#pragma once
#include "Koten/Graphics/Texture.h"



namespace KTN
{
	class GLTexture2D : public Texture2D
	{
		public:
			GLTexture2D(const TextureSpecification& p_Spec = {});
			GLTexture2D(const TextureSpecification& p_Spec, const uint8_t* p_Data, size_t p_Size);
			~GLTexture2D() override;

			void Resize(uint32_t p_Width, uint32_t p_Height) override;

			uint32_t GetWidth() const override { return m_Width; }
			uint32_t GetHeight() const override { return m_Height; }
			uint32_t GetChannels() const override { return m_Channels; }
			uint32_t GetBytesPerChannels() const override { return m_BytesPerChannels; }
			uint32_t GetMipLevels() const override { return m_MipLevels; }
			int GetSampleCount() const override { return m_Specification.Samples; }
			uint32_t GetID() const { return m_RendererID; }

			uint64_t GetEstimatedSize() const override { return (uint64_t)m_Width * m_Height * m_Channels * m_BytesPerChannels; }

			const TextureSpecification& GetSpecification() const override { return m_Specification; }

			void GenerateMipmap(CommandBuffer* p_CommandBuffer) override;

			void Bind(uint32_t p_Slot = 0) override;
			void Unbind() override;

			void SetData(const void* p_Data, size_t p_Size) override;

			bool operator== (const Texture& p_Other) const override
			{
				return m_RendererID == ((GLTexture2D&)p_Other).GetID();
			}

		private:
			void Init(const TextureSpecification& p_Spec);

		private:
			TextureSpecification m_Specification = {};

			uint32_t m_Width = 0;
			uint32_t m_Height = 0;
			uint32_t m_Channels = 0;
			uint32_t m_BytesPerChannels = 0;
			uint32_t m_RendererID = 0;
			uint32_t m_Slot = -1;
			uint32_t m_Format = 0;
			uint32_t m_InternalFormat = 0;
			uint32_t m_MipLevels = 1;
	};


} // namespace KTN
