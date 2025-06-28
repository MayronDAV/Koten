#pragma once
#include "Asset.h"
#include "Koten/Graphics/Texture.h"



namespace KTN
{
	struct KTN_API TextureImporter
	{
		static Ref<Texture2D> ImportTexture2D(AssetHandle p_Handle, const AssetMetadata& p_Metadata);

		static Ref<Texture2D> LoadTexture2D(const std::string& p_Path);
		static Ref<Texture2D> LoadTexture2D(const std::string& p_Path, const TextureSpecification& p_Spec);
	};

} // namespace KTN
