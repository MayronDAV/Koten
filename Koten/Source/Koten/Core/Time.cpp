#include "ktnpch.h"
#include "Time.h"



namespace KTN
{
	double Time::s_LastTime = 0.0f;
	double Time::s_DeltaTime = 0.0f;

	void Time::OnUpdate()
	{
		double currentTime  = GetTime();
		s_DeltaTime			= currentTime - s_LastTime;
		s_LastTime			= currentTime;
	}

	double Time::GetDeltaTime()
	{
		return s_DeltaTime;
	}

} // namespace KTN
