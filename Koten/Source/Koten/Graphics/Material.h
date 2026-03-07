#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Buffer.h"
#include "Texture.h"



namespace KTN
{
    class KTN_API Material : public Asset
    {
        public:
            std::string Name      = "None";
            glm::vec4 AlbedoColor = { 1.0f, 1.0f, 1.0f, 1.0f };
            AssetHandle Texture   = Texture2D::GetDefault();

            void Serialize(const std::string& p_Path) const;
            void SerializeBin(std::ofstream& p_Out) const;
            void DeserializeBin(std::ifstream& p_In);
            void DeserializeBin(BufferReader& p_In);

            static void DeserializeBin(std::ifstream& p_In, Buffer& p_Buffer);

            static AssetHandle GetDefault();

            Material()            = default;
            virtual ~Material()   = default;
            ASSET_CLASS_METHODS(Material)
    };


} // KTN