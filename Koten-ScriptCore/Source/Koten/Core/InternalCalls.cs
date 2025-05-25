using System;
using System.Runtime.CompilerServices;

namespace KTN
{
	public static class InternalCalls
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong p_EntityID, Type p_ComponentType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetLocalTranslation(ulong p_EntityID, out Vector3 p_Result);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetLocalTranslation(ulong p_EntityID, ref Vector3 p_Value);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyPressed(KeyCode p_Keycode);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Time_GetTime();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Time_GetDeltaTime();
	}
} // namespace KTN