#include "ktnpch.h"
#include "Koten/Core/Definitions.h"
#include "DFFont.h"
#include "Koten/Project/Project.h"

#ifdef INFINITE
	#undef INFINITE
#endif
#include <msdf-atlas-gen.h>
#include <FontGeometry.h>
#include <msdfgen.h>
#include <GlyphGeometry.h>
#include <AtlasGenerator.h>
#include <magic_enum/magic_enum.hpp>



namespace KTN
{
	namespace
	{
		static const std::filesystem::path s_FontCachePath = "Resources/FontAtlases";

		void hardmaskGenerator(const msdfgen::BitmapRef<float, 1>& p_Output, const msdf_atlas::GlyphGeometry& p_Glyph, const msdf_atlas::GeneratorAttributes& p_Attribs) 
		{
			msdfgen::rasterize(p_Output, p_Glyph.getShape(), p_Glyph.getBoxScale(), p_Glyph.getBoxTranslate(), MSDF_ATLAS_GLYPH_FILL_RULE);

			for (int y = 0; y < p_Output.height; ++y)
			{
				for (int x = 0; x < p_Output.width; ++x)
				{
					float* pixel = p_Output(x, y);
					pixel[0] = pixel[0] > 0.5f ? 1.0f : 0.0f;
				}
			}
		}

		static TextureFormat DetermineTextureFormat(FontImageType p_ImageType, FontImageFormat p_ImageFormat)
		{
			const bool floatingPoint = magic_enum::enum_name(p_ImageFormat).find("FLOAT") != std::string::npos;

			switch (p_ImageType)
			{
				case FontImageType::HARD_MASK:
				case FontImageType::SOFT_MASK:
					return floatingPoint ? TextureFormat::R32_FLOAT : TextureFormat::R8;
				case FontImageType::SDF:
				case FontImageType::PSDF:
					return floatingPoint ? TextureFormat::R32_FLOAT : TextureFormat::R8;
				case FontImageType::MSDF:
					return floatingPoint ? TextureFormat::RGB32_FLOAT : TextureFormat::RGB8;
				case FontImageType::MTSDF:
					return floatingPoint ? TextureFormat::RGBA32_FLOAT : TextureFormat::RGBA8;
				default:
					KTN_CORE_ASSERT(false, "Unknown image type");
					return TextureFormat::RGB8;
			}
		}

		static int DetermineChannelCount(FontImageType p_ImageType)
		{
			switch (p_ImageType)
			{
				case FontImageType::HARD_MASK:
				case FontImageType::SOFT_MASK:
				case FontImageType::SDF:
				case FontImageType::PSDF:
					return 1;
				case FontImageType::MSDF:
					return 3;
				case FontImageType::MTSDF:
					return 4;
				default:
					return 3;
			}
		}

		template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
		static Ref<Texture2D> CreateAtlas(
			const std::string& p_FontName, bool p_FloatingPointFormat, const std::vector<msdf_atlas::GlyphGeometry>& p_Glyphs, uint32_t p_Width, uint32_t p_Height, const DFFontConfig& p_Config)
		{
			KTN_PROFILE_FUNCTION();

			msdf_atlas::GeneratorAttributes attributes;
			attributes.config.overlapSupport = p_Config.OverlapSupport;
			attributes.scanlinePass = p_Config.ScanlinePass;

			msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(p_Width, p_Height);
			generator.setAttributes(attributes);
			generator.setThreadCount(p_Config.ThreadCount);
			generator.generate(p_Glyphs.data(), (int)p_Glyphs.size());

			msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

			TextureSpecification spec = {};
			spec.Usage = TextureUsage::TEXTURE_SAMPLED;
			spec.MinFilter = p_Config.ImageType == FontImageType::HARD_MASK ? TextureFilter::NEAREST : TextureFilter::LINEAR;
			spec.MagFilter = p_Config.ImageType == FontImageType::HARD_MASK ? TextureFilter::NEAREST : TextureFilter::LINEAR;
			spec.Width = bitmap.width;
			spec.Height = bitmap.height;
			spec.Format = DetermineTextureFormat(p_Config.ImageType, p_Config.ImageFormat);
			spec.SRGB = !p_FloatingPointFormat && p_Config.ImageType != FontImageType::HARD_MASK;
			spec.GenerateMips = false;
			spec.AnisotropyEnable = false;
			spec.DebugName = p_FontName + " Atlas Texture";

			Ref<Texture2D> texture = Texture2D::Create(spec);
			texture->SetData((void*)bitmap.pixels, (size_t)bitmap.width * bitmap.height * N * Utils::TextureFormatToBytesPerChannels(spec.Format));
			return texture;
		}

	} // namespace

	struct SDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> Glyphs;
		msdf_atlas::FontGeometry FontGeometry;
	};

	struct AtlasCacheHeader 
	{
		uint32_t Width;
		uint32_t Height;
		TextureFormat Format;
		size_t DataSize;
	};

	DFFont::~DFFont()
	{
		KTN_PROFILE_FUNCTION();

		delete m_Data;
	}

	void* DFFont::GetFontGeometry()
	{
		return (void*)&m_Data->FontGeometry;
	}

	std::vector<std::pair<glm::vec4, glm::vec4>> DFFont::CalculatePositions(const std::u32string& p_String, float p_LineSpacing, float p_Kerning) const
	{
		KTN_PROFILE_FUNCTION();

		auto& fontGeometry = m_Data->FontGeometry;
		const auto& metrics = fontGeometry.getMetrics();
		const double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);

		double x = 0.0;
		double y = 0.0;

		const auto spaceGlyph = fontGeometry.getGlyph(U' ') ?
			fontGeometry.getGlyph(U' ') :
			fontGeometry.getGlyph(0x00A0);
		const float spaceGlyphAdvance = spaceGlyph ? (float)spaceGlyph->getAdvance() : (float)metrics.ascenderY;

		std::vector<std::pair<glm::vec4, glm::vec4>> result;

		for (size_t i = 0; i < p_String.size(); i++)
		{
			char32_t character = p_String[i];
			if (character == U'\r') continue;

			if (character == U'\n')
			{
				x = 0;
				y -= fsScale * metrics.lineHeight + p_LineSpacing;
				continue;
			}

			if (character == U' ' || character == U'\t')
			{
				float advance = (character == U'\t') ?
					4.0f * ((float)fsScale * spaceGlyphAdvance + p_Kerning) :
					(float)fsScale * spaceGlyphAdvance + p_Kerning;
				x += advance;
				continue;
			}

			auto glyph = fontGeometry.getGlyph(character);
			if (!glyph) glyph = fontGeometry.getGlyph(U'?');
			if (!glyph) continue;

			double al, ab, ar, at;
			glyph->getQuadAtlasBounds(al, ab, ar, at);
			glm::vec2 texCoordMin((float)al, (float)ab);
			glm::vec2 texCoordMax((float)ar, (float)at);

			double pl, pb, pr, pt;
			glyph->getQuadPlaneBounds(pl, pb, pr, pt);
			glm::vec2 quadMin((float)pl, (float)pb);
			glm::vec2 quadMax((float)pr, (float)pt);

			quadMin *= fsScale, quadMax *= fsScale;
			quadMin += glm::vec2(x, y);
			quadMax += glm::vec2(x, y);

			result.push_back(std::make_pair(glm::vec4{ quadMin, quadMax }, glm::vec4{ texCoordMin, texCoordMax }));

			if (i < p_String.size() - 1)
			{
				double advance = glyph->getAdvance();
				fontGeometry.getAdvance(advance, character, p_String[i + 1]);
				x += fsScale * advance + p_Kerning;
			}
		}

		return result;
	}

	AssetHandle DFFont::GetDefault()
	{
		KTN_PROFILE_FUNCTION();

		auto path = Project::GetAssetFileSystemPath("Fonts/Arial/Arial Regular.ttf").string();
		if (!AssetManager::Get()->HasAsset(AssetType::Font, path))
		{
			if (!FileSystem::Exists(path) && FileSystem::Exists(FileSystem::GetAbsolute("Assets/Fonts/Arial")))
			{
				FileSystem::Copy(FileSystem::GetAbsolute("Assets/Fonts/Arial"), Project::GetAssetFileSystemPath("Fonts/Arial").string());
			}

			auto font = AssetManager::Get()->ImportAsset(AssetType::Font, path);
			if (!font)
			{
				KTN_CORE_ERROR(KTN_FONTLOG "Failed to import default font: {}", path);
				return (AssetHandle)0;
			}
			return font;
		}

		return AssetManager::Get()->GetHandleByPath(path);
	}

	Ref<DFFont> DFFont::LoadFont(AssetHandle p_Handle, const std::string& p_Font, const DFFontConfig& p_Config)
	{
		auto font = CreateRef<DFFont>();
		font->Handle = p_Handle;
		font->LoadFontImpl(p_Font, p_Config);
		if (!font->GetAtlasTexture())
		{
			KTN_CORE_ERROR(KTN_FONTLOG "Failed to load font from path: {}", p_Font);
			return nullptr;
		}

		return font;
	}

	void DFFont::LoadFontImpl(const std::string& p_Font, const DFFontConfig& p_Config) 
	{
		KTN_PROFILE_FUNCTION();

		static const auto LCG_MULTIPLIER = 6364136223846793005ull;
		static const auto LCG_INCREMENT = 1442695040888963407ull;

		m_Data = new SDFData();
		m_FontPath = p_Font;
		m_FontName = FileSystem::GetStem(p_Font);
		m_Config = p_Config;

		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		KTN_CORE_ASSERT(ft, "Failed to initialize FreeType");

		msdfgen::FontHandle* font = msdfgen::loadFont(ft, p_Font.c_str());
		if (!font) {
			KTN_CORE_ERROR(KTN_FONTLOG "Failed to load font: {}", p_Font);
			msdfgen::deinitializeFreetype(ft);
			return;
		}

		msdf_atlas::Charset charset = msdf_atlas::Charset::ASCII;

		if (p_Config.UseDefaultCharset)
		{
			static const uint32_t charsetRanges[] = {	
				0x0020, 0x00FF, // ASCII básico + Latin-1

				0x00A0, 0x00FF,  // Latin-1 Supplement
				0x0100, 0x017F,  // Latin Extended-A
				0x0180, 0x024F,  // Latin Extended-B
				0x0300, 0x036F,  // Combining Diacritical Marks

				0x00C0, 0x00C0,  // À
				0x00C1, 0x00C1,  // Á
				0x00C2, 0x00C2,  // Â
				0x00C3, 0x00C3,  // Ã
				0x00C7, 0x00C7,  // Ç
				0x00C9, 0x00C9,  // É
				0x00CD, 0x00CD,  // Í
				0x00D3, 0x00D3,  // Ó
				0x00D4, 0x00D4,  // Ô
				0x00D5, 0x00D5,  // Õ
				0x00DA, 0x00DA,  // Ú
				0x00E0, 0x00E0,  // à
				0x00E1, 0x00E1,  // á
				0x00E2, 0x00E2,  // â
				0x00E3, 0x00E3,  // ã
				0x00E7, 0x00E7,  // ç
				0x00E9, 0x00E9,  // é
				0x00ED, 0x00ED,  // í
				0x00F3, 0x00F3,  // ó
				0x00F4, 0x00F4,  // ô
				0x00F5, 0x00F5,  // õ
				0x00FA, 0x00FA,  // ú
				0
			};

			for (int range = 0; range < 8; range += 2)
			{
				for (uint32_t c = charsetRanges[range]; c <= charsetRanges[range + 1]; c++)
				{
					charset.add(c);
				}
			}
		}
		else
		{
			for (const auto& range : p_Config.CharsetRanges)
			{
				for (uint32_t c = range.first; c <= range.second; c++)
					charset.add(c);
			}
		}

		m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);
		int glyphsLoaded = -1;
		switch (p_Config.GlyphIdentifier) 
		{
			case GlyphIdentifierType::GLYPH_INDEX:
				glyphsLoaded = m_Data->FontGeometry.loadGlyphset(font, p_Config.FontScale, charset);
				break;
			case GlyphIdentifierType::UNICODE_CODEPOINT:
				glyphsLoaded = m_Data->FontGeometry.loadCharset(font, p_Config.FontScale, charset);
				break;
		}
		KTN_CORE_ASSERT(glyphsLoaded >= 0);
		KTN_CORE_INFO(KTN_FONTLOG "Loaded {} glyphs from font (out of {})", glyphsLoaded, charset.size());

		msdf_atlas::TightAtlasPacker atlasPacker;
		atlasPacker.setPixelRange(p_Config.PxRange);
		atlasPacker.setMiterLimit(p_Config.MiterLimit);

		if (p_Config.FixedScale)
			atlasPacker.setScale(p_Config.EmSize);
		else
			atlasPacker.setMinimumScale(p_Config.EmSize);

		int remaining = atlasPacker.pack(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());
		if (remaining > 0)
			KTN_CORE_WARN(KTN_FONTLOG "{} glyphs didn't fit in the atlas", remaining);

		int finalWidth, finalHeight;
		atlasPacker.getDimensions(finalWidth, finalHeight);

		if (p_Config.ImageType == FontImageType::MSDF || p_Config.ImageType == FontImageType::MTSDF) 
		{
			uint64_t coloringSeed = 0;
			bool expensiveColoring = false;
			if (expensiveColoring)
			{
				auto coloringFunc = [&](int p_I, int p_ThreadNo)
				{
					unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ p_I) + LCG_INCREMENT) * !!coloringSeed;
					m_Data->Glyphs[p_I].edgeColoring(msdfgen::edgeColoringInkTrap, p_Config.AngleThreshold, glyphSeed);
					return true;
				};

				msdf_atlas::Workload(coloringFunc, (int)m_Data->Glyphs.size()).finish(p_Config.ThreadCount);
			}
			else 
			{
				unsigned long long glyphSeed = coloringSeed;
				for (auto& glyph : m_Data->Glyphs) 
				{
					glyphSeed *= LCG_MULTIPLIER;
					glyph.edgeColoring(msdfgen::edgeColoringInkTrap, p_Config.AngleThreshold, glyphSeed);
				}
			}
		}

		const bool floatingPoint = magic_enum::enum_name(p_Config.ImageFormat).find("FLOAT") != std::string::npos;

		if (!TryLoadAtlasFromCache(floatingPoint))
		{
			const int channelCount = DetermineChannelCount(p_Config.ImageType);
			if (floatingPoint)
			{
				switch (channelCount)
				{
					case 1:
					{
						switch (p_Config.ImageType)
						{
							case FontImageType::HARD_MASK:
								m_AtlasTexture = CreateAtlas<float, float, 1, hardmaskGenerator>(p_Font, floatingPoint, m_Data->Glyphs, finalWidth, finalHeight, p_Config);
								break;
							case FontImageType::SOFT_MASK:
								m_AtlasTexture = CreateAtlas<float, float, 1, msdf_atlas::scanlineGenerator>(p_Font, floatingPoint, m_Data->Glyphs, finalWidth, finalHeight, p_Config);
								break;
							case FontImageType::PSDF:
								m_AtlasTexture = CreateAtlas<float, float, 1, msdf_atlas::psdfGenerator>(p_Font, floatingPoint, m_Data->Glyphs, finalWidth, finalHeight, p_Config);
								break;
						}
						break;
					}
					case 3:
						m_AtlasTexture = CreateAtlas<float, float, 3, msdf_atlas::msdfGenerator>(p_Font, floatingPoint, m_Data->Glyphs, finalWidth, finalHeight, p_Config);
						break;
					case 4:
						m_AtlasTexture = CreateAtlas<float, float, 4, msdf_atlas::mtsdfGenerator>(p_Font, floatingPoint, m_Data->Glyphs, finalWidth, finalHeight, p_Config);
						break;
				}
			}
			else
			{
				switch (channelCount)
				{
					case 1:
					{
						switch (p_Config.ImageType)
						{
							case FontImageType::HARD_MASK:
								m_AtlasTexture = CreateAtlas<uint8_t, float, 1, hardmaskGenerator>(p_Font, floatingPoint, m_Data->Glyphs, finalWidth, finalHeight, p_Config);
								break;
							case FontImageType::SOFT_MASK:
								m_AtlasTexture = CreateAtlas<uint8_t, float, 1, msdf_atlas::scanlineGenerator>(p_Font, floatingPoint, m_Data->Glyphs, finalWidth, finalHeight, p_Config);
								break;
							case FontImageType::PSDF:
								m_AtlasTexture = CreateAtlas<uint8_t, float, 1, msdf_atlas::psdfGenerator>(p_Font, floatingPoint, m_Data->Glyphs, finalWidth, finalHeight, p_Config);
								break;
						}
						break;
					}
					case 3:
						m_AtlasTexture = CreateAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>(p_Font, floatingPoint, m_Data->Glyphs, finalWidth, finalHeight, p_Config);
						break;
					case 4:
						m_AtlasTexture = CreateAtlas<uint8_t, float, 4, msdf_atlas::mtsdfGenerator>(p_Font, floatingPoint, m_Data->Glyphs, finalWidth, finalHeight, p_Config);
						break;
				}
			}

			SaveAtlasToCache();
		}

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);
	}

	bool DFFont::TryLoadAtlasFromCache(bool p_FloatingPointFormat)
	{
		KTN_PROFILE_FUNCTION();

		const std::string cachePath = GetCachePath();
		if (!FileSystem::Exists(cachePath))
			return false;

		std::ifstream file(cachePath, std::ios::binary);
		AtlasCacheHeader header;
		file.read((char*)&header, sizeof(header));

		std::vector<uint8_t> textureData(header.DataSize);
		file.read((char*)textureData.data(), header.DataSize);

		TextureSpecification spec = {};
		spec.Usage = TextureUsage::TEXTURE_SAMPLED;
		spec.MinFilter = m_Config.ImageType == FontImageType::HARD_MASK ? TextureFilter::NEAREST : TextureFilter::LINEAR;
		spec.MagFilter = m_Config.ImageType == FontImageType::HARD_MASK ? TextureFilter::NEAREST : TextureFilter::LINEAR;
		spec.Width = header.Width;
		spec.Height = header.Height;
		spec.Format = header.Format;
		spec.SRGB = !p_FloatingPointFormat && m_Config.ImageType != FontImageType::HARD_MASK;
		spec.GenerateMips = false;
		spec.AnisotropyEnable = false;
		spec.DebugName = m_FontName + " Atlas Texture";

		m_AtlasTexture = Texture2D::Create(spec);
		if (!m_AtlasTexture)
		{
			KTN_CORE_ERROR(KTN_FONTLOG "Failed to create font atlas texture!");
			return false;
		}
		m_AtlasTexture->SetData(textureData.data(), textureData.size());

		return true;
	}

	void DFFont::SaveAtlasToCache()
	{
		KTN_PROFILE_FUNCTION();

		FileSystem::CreateDirectories(s_FontCachePath.string());
		const std::string cachePath = GetCachePath();
		std::ofstream stream(cachePath, std::ios::binary | std::ios::trunc);
		if (!stream)
		{
			stream.close();
			KTN_CORE_ERROR(KTN_FONTLOG "Failed to cache font atlas to {}", cachePath);
			return;
		}

		AtlasCacheHeader header = {};
		header.Width = m_AtlasTexture->GetWidth();
		header.Height = m_AtlasTexture->GetHeight();
		header.Format = m_AtlasTexture->GetSpecification().Format;
		header.DataSize = m_AtlasTexture->GetEstimatedSize();

		auto textureData = m_AtlasTexture->GetData();
		stream.write((char*)&header, sizeof(AtlasCacheHeader));
		stream.write((char*)textureData.data(), header.DataSize); // TODO: Compress
	}

	std::string DFFont::GetCachePath() const
	{
		auto path = s_FontCachePath / (m_FontName + "-" + std::to_string(Handle) + ".ktfc");
		return path.string();
	}
} // namespace KTN
