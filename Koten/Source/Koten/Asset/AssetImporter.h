#pragma once
#include "Asset.h"


namespace KTN 
{
	struct KTN_API AssetImporter
	{
		static Ref<Asset> ImportAsset(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
		static Ref<Asset> ImportAssetFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data);
	};
}