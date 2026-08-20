#include "ktnpch.h"
#include "Shader.h"
#include "Koten/Core/ShaderModuleLibrary.h"

#include "Platform/OpenGL/GLShader.h"

// lib
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>



namespace KTN
{
    Ref<Shader> Shader::Create(const std::vector<AssetHandle>& p_Stages)
    {
        KTN_PROFILE_FUNCTION();

        SpirvSource source(p_Stages.size());
        for (const auto& handle : p_Stages)
        {
            auto module = ShaderModuleLibrary::Get()->GetModule(handle);
            if (module)
            {
                source[module->Type] = module->Spirv;
            }
        }

        Ref<Shader> shader = nullptr;
        if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
            shader = CreateRef<GLShader>(source);

        if (!shader)
        {
            KTN_CORE_ERROR("Unsupported API!");
            return nullptr;
        }

        shader->m_Stages = p_Stages;
        return shader;
    }

    Ref<Shader> Shader::Create(const std::initializer_list<AssetHandle>& p_Stages)
    {
        KTN_PROFILE_FUNCTION();

        SpirvSource source(p_Stages.size());
        for (const auto& handle : p_Stages)
        {
            auto module = ShaderModuleLibrary::Get()->GetModule(handle);
            if (module)
            {
                source[module->Type] = module->Spirv;
            }
        }

        Ref<Shader> shader = nullptr;
        if (Engine::Get().GetAPI() == RenderAPI::OpenGL)
            shader = CreateRef<GLShader>(source);

        if (!shader)
        {
            KTN_CORE_ERROR("Unsupported API!");
            return nullptr;
        }

        shader->m_Stages = p_Stages;
        return shader;
    }

} // namespace KTN
