#pragma once
#include "Koten/Core/Base.h"


namespace KTN
{
	class Scene;

	class KTN_API System
	{
		public:
			System() = default;
			virtual ~System() = default;

			virtual bool OnStart(Scene* p_Scene) = 0;
			virtual bool OnStop(Scene* p_Scene) = 0;
			virtual bool OnInit() = 0;
			virtual void OnUpdate(Scene* p_Scene) {};
			virtual void OnUpdate() {};

			virtual void SetPaused(bool p_Value) { m_Paused = p_Value; }

			inline const char* GetName() const
			{
				return m_DebugName;
			}

		protected:
			const char* m_DebugName = "System";

			bool m_Paused = false;
	};

} // namespace KTN
