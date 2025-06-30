#pragma once
#include "Asset.h"
#include "Koten/Graphics/MSDFFont.h"

namespace KTN
{
	class KTN_API MSDFFontImporter
	{
		public:
			static Ref<MSDFFont> ImportFont(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
	};
}