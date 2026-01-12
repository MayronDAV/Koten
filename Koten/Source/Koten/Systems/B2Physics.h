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
			void OnCreateEntity(Entity p_Entity);

			bool OnInit() override;
			void OnUpdate(Scene* p_Scene) override;
			
			const glm::vec2& GetRealGravity() const { return m_Gravity; }
			glm::vec2 GetGravity(Entity p_Entity) const;

			void SetGravity(const glm::vec2& p_Gravity);

			void SyncTransforms(Scene* p_Scene);
			
			void SetTransform(Entity p_Entity, const glm::vec3& p_Pos, float p_Rot = 0.0f);

			void MoveAndCollide(Entity p_Entity);
			void MoveAndSlide(Entity p_Entity);

			bool IsRunning() const { return m_IsRunning; }

		private:
			bool m_IsRunning = false;
			glm::vec2 m_Gravity = { 0.0f, -9.81f };

			B2WorldID m_World = {};
			int32_t m_VelocityIterations = 6;
			int32_t m_PositionIterations = 2;
	};

} // namespace KTN