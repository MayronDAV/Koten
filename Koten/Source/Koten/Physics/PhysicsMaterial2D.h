#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Asset/Asset.h"



namespace KTN
{
	class KTN_API PhysicsMaterial2D : public Asset
	{
		public:
			float Friction = 0.5f;
			float Restitution = 0.0f;
			float RestitutionThreshold = 0.5f;

			void Serialize(const std::string& p_Path) const;
			void SerializeBin(std::ofstream& p_Out) const;
			void DeserializeBin(std::ifstream& p_In);

			static AssetHandle GetDefault();

			PhysicsMaterial2D() = default;
			virtual ~PhysicsMaterial2D() = default;
			ASSET_CLASS_METHODS(PhysicsMaterial2D)
	};

} // namespace KTN