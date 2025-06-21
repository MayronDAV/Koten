#include "ktnpch.h"
#include "Font.h"

#ifdef INFINITE
	#undef INFINITE
#endif
#include <msdf-atlas-gen.h>


namespace KTN
{
	Font::Font(const std::string& p_Font)
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (ft)
		{
			msdfgen::FontHandle* font = msdfgen::loadFont(ft, p_Font.c_str());
			if (font)
			{
				msdfgen::Shape shape;
				if (msdfgen::loadGlyph(shape, font, 'C'))
				{
					shape.normalize();
					//                          max. angle
					msdfgen::edgeColoringSimple(shape, 3.0);
					//                    image width, height
					msdfgen::Bitmap<float, 3> msdf(32, 32);
					//                                 range, scale, translation
					msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
					msdfgen::savePng(msdf, "output.png");
				}
				msdfgen::destroyFont(font);
			}
			msdfgen::deinitializeFreetype(ft);
		}
	}

} // namespace KTN
