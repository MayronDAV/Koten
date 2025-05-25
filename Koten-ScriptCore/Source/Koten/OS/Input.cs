namespace KTN
{
	public class Input
	{
		public static bool IsKeyPressed(KeyCode p_Keycode)
		{
			return InternalCalls.Input_IsKeyPressed(p_Keycode);
		}
	}
} // namespace KTN