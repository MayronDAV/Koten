#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Buffer.h"
#include "Koten/Core/Definitions.h"
#include "Asset.h"

// std
#include <vector>



namespace KTN
{
    class KTN_API ShaderStage : public Asset
    {
        public:
            ShaderStage() = default;
            ~ShaderStage() = default;

            ShaderType GetStage()          const { return m_Stage; }
            const std::string& GetName()   const { return m_Name; }
            const std::string& GetPath()   const { return m_Path; }
            const std::string& GetSource() const { return m_Source; }

            ASSET_CLASS_METHODS(ShaderStage)
        private:
            ShaderType m_Stage   = ShaderType::None;
            std::string m_Name   = "None";
            std::string m_Path   = "";
            std::string m_Source = "";

            friend class ShaderStageImporter;
    };

    class KTN_API ShaderStageImporter
    {
        public:
            static Ref<ShaderStage> Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
            static Ref<ShaderStage> ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data);

            static void SaveBin(std::ofstream& p_Out, const Ref<ShaderStage>& p_Stage);
            static void LoadBin(std::ifstream& p_In, Buffer& p_Buffer);
    };
};