#pragma once
#include "Asset.h"
#include "Koten/Graphics/DFFont.h"

namespace KTN
{
	class KTN_API DFFontImporter
	{
		public:
			static Ref<DFFont> ImportFont(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
	};
}