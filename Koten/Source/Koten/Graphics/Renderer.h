#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Graphics/Texture.h"
#include "Koten/Graphics/RenderList.h"
#include "Shader.h"
#include "DescriptorSet.h"

// lib
#include <glm/glm.hpp>



namespace KTN
{
    struct RenderPassInfo
    {
        Ref<Texture2D> RenderTarget = nullptr;
        uint32_t Width              = 0;
        uint32_t Height             = 0;
        uint8_t Samples             = 1;
        glm::mat4 Projection        = {};
        glm::mat4 View              = { 1.0f };
        glm::vec4 ClearColor        = { 0.0f, 0.0f, 0.0f, 1.0f };
        bool Clear                  = true;
    };

    class KTN_API Renderer
    {
        public:
            static void Init();
            static void Shutdown();

            static void BeginFrame();
            static void EndFrame();

            static void BeginPass(const RenderPassInfo& p_PassInfo);
            static void EndPass();

            static void Submit(const RenderList& p_RenderList);
    };

} // namespace KTN