#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Graphics/Texture.h"
#include "Koten/Asset/Asset.h"

// std
#include <string>



namespace KTN
{
	struct SDFData;

	struct DFFontConfig 
	{
		FontImageType ImageType = FontImageType::MSDF;
		GlyphIdentifierType GlyphIdentifier = GlyphIdentifierType::UNICODE_CODEPOINT;
		FontImageFormat ImageFormat = FontImageFormat::PNG;
		float EmSize = 40.0f;
		double PxRange = 2.0;
		double MiterLimit = 1.0;
		double AngleThreshold = 3.0;
		double FontScale = 1.0;
		int ThreadCount = 4;
		bool ExpensiveColoring = false;
		bool FixedScale = true;

		bool OverlapSupport = true;
		bool ScanlinePass = true;

		bool UseDefaultCharset = true;
		std::vector<std::pair<uint32_t, uint32_t>> CharsetRanges = {
			{0x0020, 0x00FF},
			{0x00A0, 0x00FF},  // Latin-1 Supplement
			{0x0100, 0x017F},  // Latin Extended-A
			{0x0180, 0x024F},  // Latin Extended-B
			{0x0300, 0x036F}  // Combining Diacritical Marks
		};
	};

	// Distance Field Font
	class KTN_API DFFont : public Asset
	{
		public:
			DFFont() = default;
			~DFFont();

			void* GetFontGeometry();

			Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }

			std::string GetPath() const { return m_FontPath; }
			std::string GetName() const { return m_FontName; }
			const DFFontConfig& GetConfig() const { return m_Config; }

			//						positions, uv bounds
			std::vector<std::pair<glm::vec4, glm::vec4>> CalculatePositions(const std::u32string& p_String, float p_LineSpacing = 0.0f, float p_Kerning = 0.0f) const;

			static AssetHandle GetDefault();
			static Ref<DFFont> LoadFont(AssetHandle p_Handle, const std::string& p_Font, const DFFontConfig& p_Config = {});

			ASSET_CLASS_METHODS(Font)

		private:
			void LoadFontImpl(const std::string& p_Font, const DFFontConfig& p_Config);

			bool TryLoadAtlasFromCache(bool p_FloatingPointFormat);
			void SaveAtlasToCache();
			std::string GetCachePath() const;

		private:
			SDFData* m_Data = nullptr;
			Ref<Texture2D> m_AtlasTexture;
			DFFontConfig m_Config;

			std::string m_FontPath;
			std::string m_FontName;
	};

} // namespace KTN