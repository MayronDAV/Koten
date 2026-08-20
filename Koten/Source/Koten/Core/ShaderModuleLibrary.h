#pragma once
#include "Base.h"
#include "Definitions.h"
#include "Koten/Asset/Asset.h"
#include "Koten/Asset/ShaderStage.h"

// std
#include <unordered_set>
#include <mutex>



namespace KTN
{
    struct ShaderModule
    {
        AssetHandle Stage;
        uint64_t SourceHash;

        ShaderType Type;
        std::string Source;
        std::vector<uint32_t> Spirv;
    };

    struct StageInfo
    {
        AssetHandle Handle;
        ShaderType Stage;
        std::string Path;
        std::string Source;

        bool operator==(const StageInfo& p_Other) const {
            return (Handle == p_Other.Handle);
        }
    };

    class KTN_API ShaderModuleLibrary
    {
        public:
            static Ref<ShaderModuleLibrary> Create();

            static Ref<ShaderModuleLibrary> Get() { return s_Instance; }

        public:
            ~ShaderModuleLibrary() = default;

            void PushStage(const StageInfo& p_Info);

            void CompileStages();

            Ref<ShaderModule> GetOrCompile(const AssetHandle& p_Handle, const std::string& p_Source);
            Ref<ShaderModule> GetOrCompile(const Ref<ShaderStage>& p_Stage, const std::string& p_Source);

            Ref<ShaderModule> GetModule(const AssetHandle& p_Handle);
            bool IsModuleLoaded(const AssetHandle& p_Handle) const;

        private:
            class StageInfoHash {
                public:
                    size_t operator()(const StageInfo& p_S) const {
                        return std::hash<uint64_t>()(p_S.Handle) ^ std::hash<std::string>()(p_S.Path);
                    }
            };

            std::unordered_set<StageInfo, StageInfoHash> m_StageInfos;

            std::unordered_map<AssetHandle, Ref<ShaderModule>> m_Modules;
            std::mutex m_ModulesMutex;

            inline static Ref<ShaderModuleLibrary> s_Instance = nullptr;
    };


} // namespace KTN