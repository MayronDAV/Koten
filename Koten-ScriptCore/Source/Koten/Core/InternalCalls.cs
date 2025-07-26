using System;
using System.Runtime.CompilerServices;

namespace KTN
{
	public static class InternalCalls
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static object GetScriptInstance(ulong p_UUID);

        #region Entity
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong p_EntityID, Type p_ComponentType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_GetEntityByTag(string p_Tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_IsValid(ulong p_UUID);
        #endregion

        #region TransformComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetLocalTranslation(ulong p_EntityID, out Vector3 p_Result);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetLocalTranslation(ulong p_EntityID, ref Vector3 p_Value);
        #endregion

        #region TextRendererComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string TextRendererComponent_GetString(ulong p_EntityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetString(ulong p_EntityID, string p_String);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetFont(ulong p_EntityID, string p_Path);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string TextRendererComponent_GetFontPath(ulong p_EntityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string TextRendererComponent_GetFontName(ulong p_EntityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_GetColor(ulong p_EntityID, out Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetColor(ulong p_EntityID, ref Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_GetBgColor(ulong p_EntityID, out Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetBgColor(ulong p_EntityID, ref Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_GetCharBgColor(ulong p_EntityID, out Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetCharBgColor(ulong p_EntityID, ref Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool TextRendererComponent_GetDrawBg(ulong p_EntityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetDrawBg(ulong p_EntityID, bool p_Enable);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float TextRendererComponent_GetKerning(ulong p_EntityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetKerning(ulong p_EntityID, float p_Kerning);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float TextRendererComponent_GetLineSpacing(ulong p_EntityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetLineSpacing(ulong p_EntityID, float p_LineSpacing);
        #endregion

        #region Input
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
        #endregion

        #region Time
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Time_GetTime();

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float Time_GetDeltaTime();
        #endregion
    }
} // namespace KTN