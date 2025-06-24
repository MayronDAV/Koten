#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Graphics/Texture.h"

// std
#include <string>



namespace KTN
{
	struct MSDFData;

	class KTN_API MSDFFont
	{
		public:
			MSDFFont(const std::string& p_Font);
			~MSDFFont();

			void* GetFontGeometry();

			Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }

			std::string GetPath() const { return m_FontPath; }
			std::string GetName() const { return m_FontName; }

			static Ref<MSDFFont> GetDefault();
		private:
			MSDFData* m_Data;
			Ref<Texture2D> m_AtlasTexture;

			std::string m_FontPath;
			std::string m_FontName;
	};

} // namespace KTN