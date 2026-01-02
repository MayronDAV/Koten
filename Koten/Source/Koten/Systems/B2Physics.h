#pragma once
#include "Koten/Scene/System.h"
#include "Koten/Scene/Entity.h"

// lib
#include <glm/glm.hpp>



namespace KTN
{
	class KTN_API B2Physics : public System
	{
		public:
			B2Physics();
			~B2Physics() override;

			bool DestroyBody(const B2BodyID& p_Body);

			bool OnStart(Scene* p_Scene) override;
			bool OnStop(Scene* p_Scene) override;

			bool OnInit() override;
			void OnUpdate(Scene* p_Scene) override;

			void SetGravity(const glm::vec2& p_Gravity);

			void SyncTransforms(Scene* p_Scene);

			void MoveAndCollide(Entity p_Entity);
			void MoveAndSlide(Entity p_Entity);

		private:
			glm::vec2 m_Gravity = { 0.0f, -9.81f };

			B2WorldID m_World = {};
			int32_t m_VelocityIterations = 6;
			int32_t m_PositionIterations = 2;
	};

} // namespace KTN