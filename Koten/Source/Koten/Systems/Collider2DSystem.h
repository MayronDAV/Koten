#pragma once
#include "Koten/Scene/System.h"
#include "Koten/Collision/AABBTree.h"



namespace KTN
{
	class KTN_API Collider2DSystem : public System
	{
		public:
			Collider2DSystem();
			~Collider2DSystem() override;

			bool OnStart(Scene* p_Scene) override;
			bool OnStop(Scene* p_Scene) override;

			bool OnInit() override;
			void OnUpdate(Scene* p_Scene) override;

			void SyncBroadPhase(Scene* p_Scene, float p_Dt);

			Ref<AABBTree>& GetBroadPhase() { return m_BroadPhase; }

		private:
			Ref<AABBTree> m_BroadPhase = nullptr;
	};

} // namespace KTN