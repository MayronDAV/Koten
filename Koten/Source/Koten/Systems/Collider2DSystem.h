#pragma once
#include "Koten/Core/Base.h"
#include "Koten/Scene/System.h"
#include "Koten/Collision/AABBTree.h"
#include "Koten/Core/UUID.h"
#include "Koten/Scene/Components.h"
#include <Koten/Collision/Intersect2D.h>



namespace KTN
{
	struct ContactEvent
	{
		UUID IdA = 0;
		UUID IdB = 0;
	};

	enum ContactFlags : uint8_t
	{
		ContactFlagsStartedTouching = BIT(0),
		ContactFlagsStoppedTouching = BIT(1),
		ContactFlagsTouching = BIT(2)
	};

	struct Contact
	{
		int Index = -1;
		uint32_t Flags = 0;

		ContactEvent Event = {};
	};

	struct CachedShape
	{
		Collider2DShape Type;
		BoxShape Box;
		CircleShape Circle;

		glm::vec3 Position;
	};

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

			std::unordered_map<uint64_t, Contact> m_ContactsInfo;
			std::unordered_map<uint64_t, CachedShape> m_CachedShape;

			std::vector<ContactEvent> m_StayContacts;
			std::vector<ContactEvent> m_BeginContacts;
			std::vector<ContactEvent> m_EndContacts;
	};

} // namespace KTN