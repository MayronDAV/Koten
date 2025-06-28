#pragma once
#include "Asset.h"


namespace KTN 
{
	struct KTN_API AssetImporter
	{
		static Ref<Asset> ImportAsset(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
	};
}