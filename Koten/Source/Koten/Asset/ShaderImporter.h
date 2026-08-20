#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Core/Buffer.h"
#include "Koten/Core/Definitions.h"
#include "Asset.h"
#include "Koten/Graphics/Shader.h"
#include <string>



namespace KTN
{
    class KTN_API ShaderImporter
    {
        public:
            static Ref<Shader> Import(AssetHandle p_Handle, const AssetMetadata& p_Metadata);
            static Ref<Shader> ImportFromMemory(AssetHandle p_Handle, const AssetMetadata& p_Metadata, const Buffer& p_Data);

            static Ref<Shader> Load(const std::string& p_Path);
            static void LoadBin(std::ifstream& p_In, Buffer& p_Buffer);

            static void Save(const Ref<Shader>& p_Shader, const std::string& p_Path);
            static void Save(std::ofstream& p_Out, const Ref<Shader>& p_Shader);
    };
};