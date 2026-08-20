#include "ktnpch.h"
#include "ShaderModuleLibrary.h"
#include "Koten/Utils/HashCombiner.h"
#include "Koten/Asset/AssetManager.h"
#include "Koten/Core/JobGroup.h"
#include "Koten/Core/ThreadManager.h"

// lib
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>



namespace KTN
{
    namespace
    {
        static TBuiltInResource DefaultTBuiltInResource = {
            32,  // maxLights
            6,   // maxClipDistances
            8,   // maxCullDistances
            1,   // maxCombinedClipAndCullDistances
            2048, // maxCombinedShaderOutputResources
            4096, // maxComputeSharedMemorySize
            16,  // maxComputeWorkGroupCount
            1024, // maxComputeWorkGroupSize
            8,   // maxFragmentInputComponents
            64,  // maxImageUnits
            128, // maxImageSamples
            8,   // maxVertexOutputComponents
            8,   // maxTessControlOutputComponents
            16,  // maxTessEvaluationOutputComponents
            8,   // maxGeometryOutputComponents
            64,  // maxFragmentOutputAttachments
            8,   // maxGeometryInputComponents
            8,   // maxGeometryOutputComponents
            256, // maxFragmentCombinedOutputResources
            64,  // maxComputeWorkGroupInvocations
            16,  // maxWorkGroupSize
            1024, // maxWorkGroupCount
            8,   // maxGeometryOutputVertices
            1024, // maxGeometryTotalOutputComponents
            16,  // maxFragmentInputComponents
            16,  // maxVertexInputComponents
            1024, // maxTessControlInputComponents
            1024, // maxTessEvaluationInputComponents
            256, // maxTessControlOutputComponents
            1024, // maxTessEvaluationOutputComponents
            2048, // maxShaderStorageBufferBindings
            2048, // maxShaderStorageBufferSize
            128, // maxAtomicCounterBindings
            1024, // maxAtomicCounterBufferSize
            32,  // maxShaderImageSize
            2048, // maxShaderResourceSize
            64,  // maxShaderSamplerSize
            8,   // maxShaderConstantSize
            8,   // maxShaderPushConstantSize
            1024, // maxShaderUniformBufferSize
            128, // maxShaderStorageBufferSize
            1024, // maxShaderAtomicCounterSize
            256, // maxShaderAtomicCounterBindings
            2048, // maxShaderStorageBufferBindings
            256, // maxShaderStorageBufferSize
            2048, // maxShaderResourceSize
            128, // maxShaderSamplerSize
            128, // maxShaderSampledImageSize
            64,  // maxShaderImageSize
            8,   // maxShaderConstantSize
            16,  // maxShaderPushConstantSize
            256, // maxShaderUniformBufferSize
            2048, // maxShaderStorageBufferBindings
            256, // maxShaderStorageBufferSize
            2048, // maxShaderResourceSize
            128, // maxShaderSamplerSize
            128, // maxShaderSampledImageSize
            64,  // maxShaderImageSize
            8,   // maxShaderConstantSize
            16,  // maxShaderPushConstantSize
        };

        static EShLanguage ShaderTypeToGlslang(ShaderType p_Type)
        {
            switch (p_Type)
            {
                case ShaderType::Vertex:         return EShLangVertex;
                case ShaderType::Fragment:       return EShLangFragment;
                case ShaderType::Geometry:       return EShLangGeometry;
                case ShaderType::TessControl:    return EShLangTessControl;
                case ShaderType::TessEvaluation: return EShLangTessEvaluation;
                case ShaderType::Compute:        return EShLangCompute;
                default:
                    KTN_CORE_ERROR("Unknown shader type!")
                        return EShLangCount;
            }
        }

        static std::string ShaderTypeCachedFileExtension(ShaderType p_Type)
        {
            std::string prefix = "";
            if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
                prefix = ".opengl";
            else if (Engine::Get().GetAPI() == RenderAPI::Vulkan)
                prefix = ".vulkan";
            else
            {
                KTN_CORE_ERROR("Unsupported API!")
                    return "";
            }

            switch (p_Type)
            {
                case ShaderType::Vertex:         return prefix + ".cached.vert";
                case ShaderType::Fragment:       return prefix + ".cached.frag";
                case ShaderType::Geometry:       return prefix + ".cached.geom";
                case ShaderType::TessControl:    return prefix + ".cached.tesc";
                case ShaderType::TessEvaluation: return prefix + ".cached.tese";
                case ShaderType::Compute:        return prefix + ".cached.comp";
                default:
                    KTN_CORE_ERROR("Unknown shader type!")
                    return "";
            }
        }

        static std::vector<uint32_t> CompileGLSLToSPIRV(const std::string& p_Source, ShaderType p_Type)
        {
            KTN_PROFILE_FUNCTION();

            glslang::InitializeProcess();

            EShLanguage stage = ShaderTypeToGlslang(p_Type);
            glslang::TShader shader(stage);
            const char* shaderStrings[1];
            shaderStrings[0] = p_Source.c_str();
            shader.setStrings(shaderStrings, 1);

            std::string preamble =
                "#extension GL_ARB_shading_language_420pack : enable\n"
                "#extension GL_EXT_scalar_block_layout : enable\n";

            if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
                preamble += "#define IS_OPENGL 1\n";
            else
                preamble += "#define IS_OPENGL 0\n";

            if (Engine::Get().GetAPI() == RenderAPI::Vulkan)
                preamble += "#define IS_VULKAN 1\n";
            else
                preamble += "#define IS_VULKAN 0\n";

            shader.setPreamble(preamble.c_str());

            int ClientInputSemanticsVersion = 130;
            glslang::EShTargetClientVersion ClientVersion = glslang::EShTargetVulkan_1_3;
            glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_5;
            glslang::EShClient Client = glslang::EShClientVulkan;
            if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
            {
                ClientInputSemanticsVersion = 450;
                Client = glslang::EShClientOpenGL;
                ClientVersion = glslang::EShTargetOpenGL_450;
            }

            shader.setEnvInput(glslang::EShSourceGlsl, stage, Client, ClientInputSemanticsVersion);
            shader.setEnvClient(Client, ClientVersion);
            shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

            TBuiltInResource Resources = DefaultTBuiltInResource;
            Resources.limits.generalUniformIndexing = true;
            Resources.limits.generalVariableIndexing = true;
            Resources.limits.generalSamplerIndexing = true;
            Resources.limits.whileLoops = true;
            Resources.limits.nonInductiveForLoops = true;

            if (auto messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
                !shader.parse(&Resources, ClientInputSemanticsVersion, false, messages))
            {
                KTN_CORE_ERROR("GLSL Parsing Failed for shader: ");
                KTN_CORE_ERROR("{}", shader.getInfoLog());
                KTN_CORE_ERROR("{}", shader.getInfoDebugLog());
                KTN_CORE_ASSERT(false);
            }

            glslang::TProgram program;
            program.addShader(&shader);

            if (auto messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
                !program.link(messages))
            {
                KTN_CORE_ERROR("Program Linking Failed: ");
                KTN_CORE_ERROR("{}", program.getInfoLog());
                KTN_CORE_ERROR("{}", program.getInfoDebugLog());
                KTN_CORE_ASSERT(false);
            }

            std::vector<uint32_t> spirv;
            glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

            glslang::FinalizeProcess();
            return spirv;
        }

        static std::string ProcessIncludeFiles(const std::string& p_Path, const std::string& p_Code)
        {
            KTN_PROFILE_FUNCTION();

            auto result              = std::string();

            const char* includeToken = "#include";
            size_t pos               = p_Code.find(includeToken, 0);
            if (pos == std::string::npos)
                return p_Code;

            while (pos != std::string::npos)
            {
                if (result == std::string())
                    result   = p_Code.substr(0, pos);

                size_t eol   = p_Code.find_first_of("\r\n", pos);
                size_t start = p_Code.find("<", pos);
                size_t end   = p_Code.find(">", start);

                if (start == std::string::npos)
                {
                    start = p_Code.find("\"", pos);
                    end   = p_Code.find("\"", start);
                }

                if (start == std::string::npos || end == std::string::npos)
                {
                    KTN_CORE_ERROR("Invalid include directive in shader: {}", p_Path);
                    return p_Code;
                }

                if (start >= eol || end >= eol)
                {
                    KTN_CORE_ERROR("Not on the same line!");
                    return p_Code;
                }

                std::string includeFilepath    = p_Code.substr(start + 1, end - start - 1);
                std::filesystem::path filepath = p_Path;
                auto path                      = (filepath.parent_path() / includeFilepath).string();
                std::string includeCode        = FileSystem::ReadFile(path);

                result     += includeCode;
                pos         = p_Code.find(includeToken, end + 1);
                if (pos == std::string::npos)
                {
                    result += p_Code.substr(end + 1);
                }
                else
                {
                    result += p_Code.substr(end + 1, pos - end - 1);
                }
            }

            return result;
        }

    } // namespace 

    Ref<ShaderModuleLibrary> KTN::ShaderModuleLibrary::Create()
    {
        KTN_PROFILE_FUNCTION();

        if (!s_Instance)
            s_Instance = CreateRef<ShaderModuleLibrary>();

        return s_Instance;
    }

    void ShaderModuleLibrary::PushStage(const StageInfo& p_Info)
    {
        KTN_PROFILE_FUNCTION();

        m_StageInfos.insert(p_Info);
    }

    void ShaderModuleLibrary::CompileStages()
    {
        KTN_PROFILE_FUNCTION();

        const std::string path = "Assets/Shaders/Stages.ktbin";
        FileSystem::CreateDirectories("Assets/Shaders");

        std::ifstream in(path, std::ios::binary);
        if (in)
        {
            size_t count = 0;
            in.read(reinterpret_cast<char*>(&count), sizeof(count));

            for (size_t i = 0; i < count; i++)
            {
                auto module = CreateRef<ShaderModule>();

                in.read(reinterpret_cast<char*>(&module->Stage), sizeof(module->Stage));
                in.read(reinterpret_cast<char*>(&module->Type), sizeof(module->Type));
                in.read(reinterpret_cast<char*>(&module->SourceHash), sizeof(module->SourceHash));

                size_t size = 0;
                in.read(reinterpret_cast<char*>(&size), sizeof(size));
                module->Spirv.resize(size);
                in.read(reinterpret_cast<char*>(module->Spirv.data()), size * sizeof(uint32_t));

                m_Modules[module->Stage] = module;
            }
        }

        bool updated = false;
        auto stagesGroup = JobGroup::Create();
        for (const auto& info : m_StageInfos)
        {
            ThreadManager::Get().ScheduleJob(stagesGroup, [&]()
            {
                uint64_t hash = HashString(info.Source);

                {
                    std::lock_guard<std::mutex> lock(m_ModulesMutex);

                    if (!m_Modules.empty())
                    {
                        auto it = m_Modules.find(info.Handle);
                        if (it != m_Modules.end())
                        {
                            if (it->second->SourceHash == hash)
                                return it->second;
                        }
                    }
                }

                updated                    = true;
                auto module                = CreateRef<ShaderModule>();
                module->Stage              = info.Handle;
                module->SourceHash         = hash;
                module->Type               = info.Stage;
                module->Source             = ProcessIncludeFiles(info.Path, info.Source);
                module->Spirv              = CompileGLSLToSPIRV(module->Source, module->Type);

                {
                    std::lock_guard<std::mutex> lock(m_ModulesMutex);
                    m_Modules.insert({ info.Handle, module });
                }

                KTN_CORE_INFO("Compiled shader stage {}", info.Path);
            });
        }

        stagesGroup->Wait();

        if (updated)
        {
            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            if (!out)
            {
                KTN_CORE_ERROR("Failed to create stages bin at {}", path);
                return;
            }

            size_t size = m_Modules.size();
            out.write(reinterpret_cast<const char*>(&size), sizeof(size));

            for (const auto& [handle, module] : m_Modules)
            {
                out.write(reinterpret_cast<const char*>(&module->Stage), sizeof(module->Stage));
                out.write(reinterpret_cast<const char*>(&module->Type), sizeof(module->Type));
                out.write(reinterpret_cast<const char*>(&module->SourceHash), sizeof(module->SourceHash));

                size_t size = module->Spirv.size();
                out.write(reinterpret_cast<const char*>(&size), sizeof(size));
                out.write(reinterpret_cast<const char*>(module->Spirv.data()), size * sizeof(uint32_t));
            }
        }
    }

    Ref<ShaderModule> ShaderModuleLibrary::GetOrCompile(const AssetHandle& p_Handle, const std::string& p_Source)
    {
        KTN_PROFILE_FUNCTION();

        auto shaderStage = AssetManager::Get()->GetAsset<ShaderStage>(p_Handle);
        return GetOrCompile(shaderStage, p_Source);
    }

    Ref<ShaderModule> ShaderModuleLibrary::GetOrCompile(const Ref<ShaderStage>& p_Stage, const std::string& p_Source)
    {
        KTN_PROFILE_FUNCTION();

        uint64_t hash = HashString(p_Source);

        auto it = m_Modules.find(p_Stage->Handle);
        if (it != m_Modules.end())
        {
            if (it->second->SourceHash == hash)
                return it->second;
        }

        auto module                = CreateRef<ShaderModule>();
        module->Stage              = p_Stage->Handle;
        module->SourceHash         = hash;
        module->Type               = p_Stage->GetStage();
        module->Source             = ProcessIncludeFiles(p_Stage->GetPath(), p_Source);
        module->Spirv              = CompileGLSLToSPIRV(module->Source, module->Type);

        m_Modules[p_Stage->Handle] = module;
        return module;
    }

    bool ShaderModuleLibrary::IsModuleLoaded(const AssetHandle& p_Handle) const
    {
        return m_Modules.find(p_Handle) != m_Modules.end();
    }

    Ref<ShaderModule> ShaderModuleLibrary::GetModule(const AssetHandle& p_Handle)
    {
        KTN_PROFILE_FUNCTION();

        auto it = m_Modules.find(p_Handle);
        if (it != m_Modules.end())
        {
            return it->second;
        }

        return nullptr;
    }
}
