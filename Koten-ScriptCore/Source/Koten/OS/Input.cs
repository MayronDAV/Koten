namespace KTN
{
	public class Input
	{
		public static bool IsKeyPressed(KeyCode p_Keycode)
		{
			return InternalCalls.Input_IsKeyPressed(p_Keycode);
		}

		public static bool IsKeyReleased(KeyCode p_Keycode)
		{
			return InternalCalls.Input_IsKeyReleased(p_Keycode);
		}

		public static bool IsMouseButtonPressed(MouseCode p_MouseButton)
		{
			return InternalCalls.Input_IsMouseButtonPressed(p_MouseButton);
		}

		public static bool IsMouseButtonReleased(MouseCode p_MouseButton)
		{
			return InternalCalls.Input_IsMouseButtonReleased(p_MouseButton);
		}

		public static float GetMouseX()
		{
			return InternalCalls.Input_GetMouseX();
		}

		public static float GetMouseY()
		{
			return InternalCalls.Input_GetMouseY();
		}

		public static Vector2 GetMousePosition()
		{
			InternalCalls.Input_GetMousePosition(out Vector2 ret);
			return ret;
		}

		public static bool IsControllerButtonPressed(int p_ControllerIndex, GamepadCode p_Button)
		{
			return InternalCalls.Input_IsControllerButtonPressed(p_ControllerIndex, p_Button);
		}

		public static bool IsControllerButtonReleased(int p_ControllerIndex, GamepadCode p_Button)
		{
			return InternalCalls.Input_IsControllerButtonReleased(p_ControllerIndex, p_Button);
		}

		public static bool IsControllerButtonHeld(int p_ControllerIndex, GamepadCode p_Button)
		{
			return InternalCalls.Input_IsControllerButtonHeld(p_ControllerIndex, p_Button);
		}

		public static bool IsControllerConnected(int p_ControllerIndex)
		{
			return InternalCalls.Input_IsControllerConnected(p_ControllerIndex);
		}

		public static float GetControllerAxis(int p_ControllerIndex, GamepadCode p_Axis)
		{
			return InternalCalls.Input_GetControllerAxis(p_ControllerIndex, p_Axis);
		}

		public static void GetConnectedControllerIDs(int[] p_Array)
		{
			InternalCalls.Input_GetConnectedControllerIDs(p_Array);
		}

		public static string GetControllerName(int p_ControllerIndex)
		{
			return InternalCalls.Input_GetControllerName(p_ControllerIndex);
		}
	}
} // namespace KTN