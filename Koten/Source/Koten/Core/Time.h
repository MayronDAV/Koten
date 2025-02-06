#pragma once
#include "Koten/Core/Base.h"



namespace KTN
{
	class KTN_API Time
	{
		public:
			static void OnUpdate();

			static double GetTime();

			static double GetDeltaTime();

		private:
			static double s_LastTime;
			static double s_DeltaTime;
	};

} // namespace KTN