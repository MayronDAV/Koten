#include "ktnpch.h"
#include "GLBase.h"
#include "GLUtils.h"
#include "GLRendererAPI.h"
#include "GLTexture.h"
#include "Koten/Graphics/Renderer.h"
#include "Koten/Graphics/RendererCommand.h"
#include "Koten/Graphics/Pipeline.h"
#include "Koten/Core/TaskManager.h"

// lib
#include <stb/stb_image_write.h>




namespace KTN
{
    namespace
    {
        static void FlipVertical(uint8_t* p_Data, uint32_t p_Width, uint32_t p_Height, uint32_t p_Channels)
        {
            KTN_PROFILE_FUNCTION_LOW();

            size_t stride       = p_Width * p_Channels;

            std::vector<uint8_t> temp(stride);

            for (uint32_t y = 0; y < p_Height / 2; y++)
            {
                uint8_t* top    = p_Data + y * stride;
                uint8_t* bottom = p_Data + (p_Height - 1 - y) * stride;

                memcpy(temp.data(), top, stride);
                memcpy(top, bottom, stride);
                memcpy(bottom, temp.data(), stride);
            }
        }
    }

    GLRendererAPI::GLRendererAPI()
    {
        KTN_PROFILE_FUNCTION_LOW();

        m_CommandBuffer = CommandBuffer::Create();
        m_CommandBuffer->Init();

        // MaxSamples
        GLCall(glGetIntegerv(GL_MAX_SAMPLES, &m_Capabilities.MaxSamples));

        // SamplerAnisotropy
        GLCall(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_Capabilities.MaxAnisotropy));
        m_Capabilities.SamplerAnisotropy = (m_Capabilities.MaxAnisotropy > 1.0f);

        // MaxTextureUnits
        GLCall(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_Capabilities.MaxTextureUnits));

        // WideLines
        GLfloat lineWidthRange[2];
        GLCall(glGetFloatv(GL_LINE_WIDTH_RANGE, lineWidthRange));
        m_Capabilities.WideLines = (lineWidthRange[1] > 1.0f);
        m_Capabilities.MaxLineWidth = lineWidthRange[1];

        m_Capabilities.SupportCompute = GLAD_GL_ARB_compute_shader == 1;

        m_Capabilities.SupportGeometry = GLAD_GL_ARB_geometry_shader4 == 1;

        GLCall(glEnable(GL_DEPTH_TEST));
        GLCall(glEnable(GL_STENCIL_TEST));
        GLCall(glEnable(GL_CULL_FACE));
        GLCall(glDepthFunc(GL_LEQUAL));
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        GLCall(glBlendEquation(GL_FUNC_ADD));

        GLCall(glGenFramebuffers(1, &m_FBO));
    }

    void GLRendererAPI::ClearColor(const glm::vec4& p_Color)
    {
        KTN_PROFILE_FUNCTION_LOW();

        GLCall(glClearColor(p_Color.r, p_Color.g, p_Color.b, p_Color.a));
        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
    }

    void GLRendererAPI::ClearRenderTarget(const Ref<Texture2D>& p_Texture, int p_Value)
    {
        KTN_PROFILE_FUNCTION_LOW();

        KTN_CORE_VERIFY(p_Texture != nullptr);

        auto id = As<Texture2D, GLTexture2D>(p_Texture)->GetID();

        auto format = p_Texture->GetSpecification().Format;
        if (p_Texture->IsColorAttachment())
        {
            GLCall(glClearTexImage(id, 0, GLUtils::TextureFormatToGLFormat(format), GLUtils::TextureFormatToGLType(format), &p_Value));
        }
        else if (p_Texture->IsDepthStencilAttachment())
        {
            float value = 1.0f;
            GLCall(glClearTexImage(id, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &value));

            if (p_Texture->IsStencil())
            {
                GLint clearStencil = 0;
                GLCall(glClearTexImage(id, 0, GL_STENCIL_INDEX, GL_INT, &clearStencil));
            }
        }
    }

    void GLRendererAPI::ClearRenderTarget(const Ref<Texture2D>& p_Texture, const glm::vec4& p_Value)
    {
        KTN_PROFILE_FUNCTION_LOW();

        KTN_CORE_VERIFY(p_Texture != nullptr);

        auto id = As<Texture2D, GLTexture2D>(p_Texture)->GetID();

        auto format = p_Texture->GetSpecification().Format;
        if (p_Texture->IsColorAttachment())
        {
            GLCall(glClearTexImage(id, 0, GLUtils::TextureFormatToGLFormat(format), GLUtils::TextureFormatToGLType(format), &p_Value[0]));
        }
        else if (p_Texture->IsDepthStencilAttachment())
        {
            float value = 1.0f;
            GLCall(glClearTexImage(id, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &value));

            if (p_Texture->IsStencil())
            {
                GLint clearStencil = 0;
                GLCall(glClearTexImage(id, 0, GL_STENCIL_INDEX, GL_INT, &clearStencil));
            }
        }
    }

    bool GLRendererAPI::SaveTextureToFile(const Ref<Texture2D>& p_Texture, const std::string& p_Path, bool p_Async)
    {
        KTN_PROFILE_FUNCTION_LOW();

        if (!p_Texture)
        {
            KTN_CORE_ERROR(KTN_GLLOG "Texture is null!");
            return false;
        }

        auto spec        = p_Texture->GetSpecification();
        auto format      = GLUtils::TextureFormatToGLFormat(spec.Format);
        auto width       = p_Texture->GetWidth();
        auto height      = p_Texture->GetHeight();
        auto channels    = p_Texture->GetChannels();

        std::vector<uint8_t> pixels(width * height * channels);
        auto glTexture   = As<Texture2D, GLTexture2D>(p_Texture);

        GLCall(glGetTextureImage(glTexture->GetID(), 0, format, GL_UNSIGNED_BYTE, static_cast<GLsizei>(pixels.size()), pixels.data()));

        auto path        = FileSystem::ReplaceExtension(p_Path, ".png");
        //FileSystem::CreateDirectories(path);

        if (p_Async)
        {
            ThreadManager::Get().ScheduleJob([tempData = std::move(pixels), path, width, height, channels]() mutable
            {
                FlipVertical(tempData.data(), width, height, channels);

                int32_t resWrite = stbi_write_png(
                    path.c_str(),
                    width, height, channels,
                    tempData.data(), width * channels);

                if (!resWrite)
                {
                    KTN_CORE_ERROR(KTN_GLLOG "Failed to save texture to path: {}!", path);
                    return;
                }

                KTN_CORE_INFO(KTN_GLLOG "Texture saved to path: {}", path);
            });

            return true;
        }

        FlipVertical(pixels.data(), width, height, channels);

        int32_t resWrite = stbi_write_png(
            path.c_str(),
            width, height, channels,
            pixels.data(), width * channels);

        if (!resWrite)
        {
            KTN_CORE_ERROR(KTN_GLLOG "Failed to save texture to path: {}!", path);
            return false;
        }

        KTN_CORE_INFO(KTN_GLLOG "Texture saved to path: {}", path);
        return true;
    }

    void* GLRendererAPI::ReadPixel(const Ref<Texture2D>& p_Texture, uint32_t p_X, uint32_t p_Y)
    {
        KTN_PROFILE_FUNCTION_LOW();

        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));

        uint32_t id = As<Texture2D, GLTexture2D>(p_Texture)->GetID();
        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0));

        static const GLenum buffers = GL_COLOR_ATTACHMENT0;
        GLCall(glDrawBuffers(1, &buffers));

        GLCall(glReadBuffer(GL_COLOR_ATTACHMENT0));

        void* pixel;
        GLCall(glReadPixels(p_X, p_Y, 1, 1,
            GLUtils::TextureFormatToGLFormat(p_Texture->GetSpecification().Format),
            GLUtils::TextureFormatToGLType(p_Texture->GetSpecification().Format), &pixel));

        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        return pixel;
    }

    void GLRendererAPI::Begin()
    {
        KTN_PROFILE_FUNCTION_LOW();

        GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    void GLRendererAPI::End()
    {
    }

} // namespace KTN