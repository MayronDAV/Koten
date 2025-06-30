#pragma once

#include "Koten/Core/UUID.h"

// std
#include <string>


namespace KTN
{
	using AssetHandle = UUID;

	enum class AssetType : uint16_t
	{
		None = 0,
		Scene,
		Font,
		Texture2D
	};

	KTN_API const char* GetAssetTypeName(AssetType p_Type);
	KTN_API AssetType GetAssetTypeFromName(const char* p_Name);

	#define ASSET_CLASS_METHODS(type)															\
			static AssetType GetStaticType() { return AssetType::type; }						\
			virtual AssetType GetType() const override { return GetStaticType(); }

	class KTN_API Asset
	{
		public:
			AssetHandle Handle;

			virtual AssetType GetType() const = 0;
	};

	struct KTN_API AssetMetadata
	{
		AssetType Type = AssetType::None;
		std::string FilePath;
		void* AssetData = nullptr; // Pointer to the actual asset data (e.g., for fonts, textures, etc.)

		operator bool() const { return Type != AssetType::None; }
	};

} // namespace KTN