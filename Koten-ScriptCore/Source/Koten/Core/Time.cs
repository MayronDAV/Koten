namespace KTN
{
	public static class Time
	{
		public static float CurTime 
		{ 
			get 
			{ 
				return InternalCalls.Time_GetTime(); 
			}
		}

		public static float DeltaTime
		{
			get
			{
				return InternalCalls.Time_GetDeltaTime();
			}
		}
	}

} // namespace KTN