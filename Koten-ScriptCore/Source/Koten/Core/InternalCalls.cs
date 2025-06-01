using System;
using System.Runtime.CompilerServices;

namespace KTN
{
	public static class InternalCalls
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static object GetScriptInstance(ulong p_UUID);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong p_EntityID, Type p_ComponentType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_GetEntityByTag(string p_Tag);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetLocalTranslation(ulong p_EntityID, out Vector3 p_Result);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetLocalTranslation(ulong p_EntityID, ref Vector3 p_Value);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyPressed(KeyCode p_Keycode);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyReleased(KeyCode p_Keycode);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonPressed(MouseCode p_MouseCode);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsMouseButtonReleased(MouseCode p_MouseCode);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsControllerButtonPressed(int p_ControllerIndex, GamepadCode p_Button);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsControllerButtonReleased(int p_ControllerIndex, GamepadCode p_Button);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsControllerButtonHeld(int p_ControllerIndex, GamepadCode p_Button);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsControllerConnected(int p_ControllerIndex);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Input_GetControllerAxis(int p_ControllerIndex, GamepadCode p_Axis);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Input_GetMouseX();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Input_GetMouseY();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Input_GetMousePosition(out Vector2 p_Result);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern  static void Input_GetConnectedControllerIDs(int[] p_Array);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static string Input_GetControllerName(int p_ControllerIndex);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Time_GetTime();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Time_GetDeltaTime();
	}
} // namespace KTN