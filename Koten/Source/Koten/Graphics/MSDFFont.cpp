#include "ktnpch.h"
#include "Koten/Core/Definitions.h"
#include "MSDFFont.h"

#ifdef INFINITE
	#undef INFINITE
#endif
#include <msdf-atlas-gen.h>
#include <FontGeometry.h>
#include <GlyphGeometry.h>



namespace KTN
{
	namespace
	{
		template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
		static Ref<Texture2D> CreateAndCacheAtlas(
			const std::string& p_FontName, float p_FontSize, const std::vector<msdf_atlas::GlyphGeometry>& p_Glyphs,
			const msdf_atlas::FontGeometry& p_FontGeometry, uint32_t p_Width, uint32_t p_Height)
		{
			KTN_PROFILE_FUNCTION();

			msdf_atlas::GeneratorAttributes attributes;
			attributes.config.overlapSupport = true;
			attributes.scanlinePass = true;

			msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(p_Width, p_Height);
			generator.setAttributes(attributes);
			generator.setThreadCount(8);
			generator.generate(p_Glyphs.data(), (int)p_Glyphs.size());

			msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

			TextureSpecification spec;
			spec.AnisotropyEnable = false;
			spec.Width = bitmap.width;
			spec.Height = bitmap.height;
			spec.Format = TextureFormat::RGB8;
			spec.GenerateMips = false;

			//return Texture2D::Create(spec, (void*)bitmap.pixels, bitmap.width * bitmap.height * 3);
			Ref<Texture2D> texture = Texture2D::Create(spec);
			texture->SetData((void*)bitmap.pixels, (size_t)bitmap.width * bitmap.height * 3);
			return texture;
		}
	} // namespace

	struct MSDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> Glyphs;
		msdf_atlas::FontGeometry FontGeometry;
	};

	MSDFFont::MSDFFont(const std::string& p_Font)
		: m_Data(new MSDFData()), m_FontPath(p_Font), m_FontName(FileSystem::GetStem(p_Font))
	{
		KTN_PROFILE_FUNCTION();

		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		KTN_CORE_ASSERT(ft);

		msdfgen::FontHandle* font = msdfgen::loadFont(ft, p_Font.c_str());
		if (!font)
		{
			KTN_CORE_ERROR("Failed to load font: {}", p_Font);
			return;
		}

		struct CharsetRange
		{
			uint32_t Begin, End;
		};

		// From imgui_draw.cpp
		static const CharsetRange charsetRanges[] =
		{
			{ 0x0020, 0x00FF }
		};

		msdf_atlas::Charset charset;
		for (CharsetRange range : charsetRanges)
		{
			for (uint32_t c = range.Begin; c <= range.End; c++)
				charset.add(c);
		}

		double fontScale = 1.0;
		m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);
		int glyphsLoaded = m_Data->FontGeometry.loadCharset(font, fontScale, charset);
		KTN_CORE_INFO("Loaded {} glyphs from font (out of {})", glyphsLoaded, charset.size());


		double emSize = 40.0;

		msdf_atlas::TightAtlasPacker atlasPacker;
		// atlasPacker.setDimensionsConstraint()
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setInnerPixelPadding(0);
		atlasPacker.setScale(emSize);
		int remaining = atlasPacker.pack(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());
		KTN_CORE_ASSERT(remaining == 0);

		int width, height;
		atlasPacker.getDimensions(width, height);
		emSize = atlasPacker.getScale();

		#define DEFAULT_ANGLE_THRESHOLD 3.0
		#define LCG_MULTIPLIER 6364136223846793005ull
		#define LCG_INCREMENT 1442695040888963407ull
		#define THREAD_COUNT 8
		// if MSDF || MTSDF

		uint64_t coloringSeed = 0;
		bool expensiveColoring = false;
		if (expensiveColoring)
		{
			msdf_atlas::Workload([&glyphs = m_Data->Glyphs, &coloringSeed](int i, int threadNo) -> bool {
				unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
				}, m_Data->Glyphs.size()).finish(THREAD_COUNT);
		}
		else {
			unsigned long long glyphSeed = coloringSeed;
			for (msdf_atlas::GlyphGeometry& glyph : m_Data->Glyphs)
			{
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}

		m_AtlasTexture = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", (float)emSize, m_Data->Glyphs, m_Data->FontGeometry, width, height);

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);
	}

	MSDFFont::~MSDFFont()
	{
		KTN_PROFILE_FUNCTION();

		delete m_Data;
	}

	void* MSDFFont::GetFontGeometry()
	{
		return (void*)&m_Data->FontGeometry;
	}

	Ref<MSDFFont> MSDFFont::GetDefault()
	{
		KTN_PROFILE_FUNCTION();

		static Ref<MSDFFont> DefaultFont;
		if (!DefaultFont)
			DefaultFont = CreateRef<MSDFFont>("Assets/Fonts/OpenSans/OpenSans-Regular.ttf");

		return DefaultFont;
	}

} // namespace KTN
