#include "ktnpch.h"
#include "DebugRenderer.h"
#include "Renderer.h"

// lib
#include <box2d/types.h>



namespace KTN
{
	void DebugRenderer::DrawCollider2D(Entity p_Entity, const glm::vec4& p_Color)
	{
		KTN_PROFILE_FUNCTION();

		auto& tc = p_Entity.GetComponent<TransformComponent>();
		glm::vec3 tpos = tc.GetWorldTranslation();
		glm::vec3 tscale = tc.GetWorldScale();
		glm::vec3 trot = tc.GetWorldRotation();

		if (p_Entity.HasComponent<BoxCollider2DComponent>())
		{
			auto& bc2d = p_Entity.GetComponent<BoxCollider2DComponent>();
			glm::vec3 translation = tpos + glm::vec3(bc2d.Offset, 0.0f);
			glm::vec3 scale = tscale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

			glm::mat4 transform = glm::translate(glm::mat4(1.0f), tpos)
				* glm::rotate(glm::mat4(1.0f), trot.z, glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, 0.001f))
				* glm::scale(glm::mat4(1.0f), scale);

			DebugRenderer::DrawSquare(transform, p_Color);
		}

		if (p_Entity.HasComponent<CircleCollider2DComponent>())
		{
			auto& cc2d = p_Entity.GetComponent<CircleCollider2DComponent>();
			glm::vec3 translation = tpos + glm::vec3(cc2d.Offset, 0.001f);
			glm::vec3 scale = tscale * glm::vec3(cc2d.Radius * 2.0f);

			glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
				* glm::scale(glm::mat4(1.0f), scale);

			DebugRenderer::DrawCircle(transform, p_Color, (int)p_Entity.GetHandle());
		}
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

		auto& settings = Engine::Get().GetSettings();

		RenderCommand command = {};
		command.EntityID = p_Entity;
		command.Transform = p_Transform;
		command.Type = RenderType::Line;
		command.Line.Primitive = true;
		command.Line.Color = p_Color;
		command.Line.Width = settings.DebugLineWidth;
		for (int i = 0; i < 4; ++i)
		{
			command.Line.Start = vertices[i];
			command.Line.End = vertices[i < 3 ? i + 1 : 0];

			Renderer::Submit(command);
		}
	}

	void DebugRenderer::DrawCircle(const glm::mat4& p_Transform, const glm::vec4& p_Color, int p_Entity)
	{
		KTN_PROFILE_FUNCTION();

		auto& settings = Engine::Get().GetSettings();

		RenderCommand command = {};
		command.EntityID = p_Entity;
		command.Type = RenderType::R2D;
		command.Transform = p_Transform;
		command.Render2D.Type = RenderType2D::Circle;
		command.Render2D.Color = p_Color;
		command.Render2D.Thickness = settings.DebugCircleThickness;
		command.Render2D.Fade = 0.005f;

		Renderer::Submit(command);
	}

} // namespace KTN
