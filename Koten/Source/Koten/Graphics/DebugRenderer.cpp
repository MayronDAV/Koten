#include "ktnpch.h"
#include "DebugRenderer.h"
#include "Renderer.h"

// lib
#include <box2d/types.h>



namespace KTN
{
    void DebugRenderer::DrawHairLine(const glm::vec3& p_Start, const glm::vec3& p_End, const glm::vec4& p_Color, int p_Entity)
    {
        KTN_PROFILE_FUNCTION();

        RenderCommand command            = {};
        command.EntityID                 = p_Entity;

        LineCommand line                 = {};
        line.Primitive                   = true;
        line.Color                       = p_Color;
        line.Width                       = 2.0f;
        line.Start                       = p_Start;
        line.End                         = p_End;

        s_RenderList->Submit(command);
    }

    void DebugRenderer::DrawCollider2D(Entity p_Entity, const glm::vec4& p_Color)
    {
        KTN_PROFILE_FUNCTION();

        auto& tc         = p_Entity.GetComponent<TransformComponent>();
        glm::vec3 tpos   = tc.GetWorldTranslation();
        glm::vec3 tscale = tc.GetWorldScale();
        glm::vec3 trot   = tc.GetWorldRotation();

        if (p_Entity.HasComponent<BodyShape2DComponent>())
        {
            auto& collider = p_Entity.GetComponent<BodyShape2DComponent>();

            if (collider.Shape == Shape2D::Rect)
            {
                glm::vec3 translation = tpos + glm::vec3(collider.Offset, 0.0f);
                glm::vec3 scale       = tscale * glm::vec3(collider.Size * 2.0f, 1.0f);

                glm::mat4 transform   = glm::translate(glm::mat4(1.0f), tpos)
                    * glm::rotate(glm::mat4(1.0f), trot.z, glm::vec3(0.0f, 0.0f, 1.0f))
                    * glm::translate(glm::mat4(1.0f), glm::vec3(collider.Offset, 0.001f))
                    * glm::scale(glm::mat4(1.0f), scale);

                DebugRenderer::DrawSquare(transform, p_Color);
            }

            if (collider.Shape == Shape2D::Circle)
            {
                glm::vec3 translation = tpos + glm::vec3(collider.Offset, 0.001f);
                glm::vec3 scale = tscale * glm::vec3(collider.Size.x * 2.0f);

                glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                    * glm::scale(glm::mat4(1.0f), scale);

                DebugRenderer::DrawCircle(transform, p_Color, (int)p_Entity.GetHandle());
            }
        
        }
    }

    void DebugRenderer::DrawAABB(Entity p_Entity, const glm::vec2& p_Min, const glm::vec2& p_Max, const glm::vec4& p_Color)
    {
        KTN_PROFILE_FUNCTION();

        glm::vec2 center = (p_Min + p_Max) * 0.5f;
        glm::vec2 size   = (p_Max - p_Min);

        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.001f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

        DebugRenderer::DrawSquare(transform, p_Color, (int)p_Entity.GetHandle());
    }

    void DebugRenderer::DrawSquare(const glm::mat4& p_Transform, const glm::vec4& p_Color, int p_Entity)
    {
        KTN_PROFILE_FUNCTION();

        static const glm::vec3 vertices[4] = {
            { -0.5f, -0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            {  0.5f,  0.5f, 0.0f },
            { -0.5f,  0.5f, 0.0f }
        };

        auto& settings        = Engine::Get().GetSettings();

        RenderCommand command = {};
        command.EntityID      = p_Entity;
        command.Transform     = p_Transform;

        LineCommand line      = {};
        line.Primitive        = true;
        line.Color            = p_Color;
        line.Width            = settings.DebugLineWidth;
        for (int i = 0; i < 4; ++i)
        {
            line.Start        = vertices[i];
            line.End          = vertices[i < 3 ? i + 1 : 0];

            command.Command   = line;
            s_RenderList->Submit(command);
        }
    }

    void DebugRenderer::DrawCircle(const glm::mat4& p_Transform, const glm::vec4& p_Color, int p_Entity)
    {
        KTN_PROFILE_FUNCTION();

        auto& settings                       = Engine::Get().GetSettings();

        RenderCommand command                = {};
        command.EntityID                     = p_Entity;
        command.Transform                    = p_Transform;

        SpriteCommand sprite                 = {};
        sprite.Type                          = RenderType2D::Circle;
        sprite.Color                         = p_Color;
        sprite.Thickness                     = settings.DebugCircleThickness;
        sprite.Fade                          = 0.005f;

        command.Command                      = sprite;
        s_RenderList->Submit(command);
    }

    void DebugRenderer::Begin(RenderList* p_RenderList)
    {
        KTN_PROFILE_FUNCTION();

        KTN_CORE_ASSERT(p_RenderList, "RenderList is null!");
        s_RenderList = p_RenderList;
    }

    void DebugRenderer::End()
    {
        KTN_PROFILE_FUNCTION();

        s_RenderList = nullptr;
    }

} // namespace KTN
