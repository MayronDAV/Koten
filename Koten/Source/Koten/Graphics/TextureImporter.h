#pragma once
#include "Texture.h"



namespace KTN
{
	struct KTN_API TextureImporter
	{
		static Ref<Texture2D> LoadTexture2D(const std::string& p_Path);
		static Ref<Texture2D> LoadTexture2D(const std::string& p_Path, const TextureSpecification& p_Spec);
	};

} // namespace KTN
