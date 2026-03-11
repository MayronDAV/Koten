using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;


namespace KTN
{
    [StructLayout(LayoutKind.Sequential)]
    internal struct ObjectHandle
    {
        public ulong ID;
        public ulong SceneHandle;
        public Int32 Type;

        public ObjectHandle(ulong p_ID, ulong p_SceneHandle)
        {
            ID = p_ID;
            SceneHandle = p_SceneHandle;
            Type = 0;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct SceneHandle
    {
        public ulong Handle;

        // Configuration Data

        public byte UsePhysics2D;
    }


    public static class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong p_UUID);

        #region Window

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetTitle(string p_Title);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetVsync(bool p_Value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Window_IsVsync();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Window_IsMaximized();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Window_IsMinimized();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static UInt32 Window_GetWidth();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static UInt32 Window_GetHeight();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetWidth(UInt32 p_Width);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetHeight(UInt32 p_Height);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Window_Resize(UInt32 p_Width, UInt32 p_Height);

        #endregion

        #region Object
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ObjectHandle Object_Instantiate(ObjectHandle p_Handle, ref Vector3 p_Postion, ref Vector3 p_Rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Object_Destroy(ObjectHandle p_Handle);
        #endregion

        #region AssetManager
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ObjectHandle AssetManager_FindWithPath(string p_Path);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AssetManager_IsAssetHandleValid(ulong p_AssetHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool AssetManager_IsAssetLoaded(ulong p_AssetHandle);

        #endregion

        #region GameObject
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool GameObject_HasComponent(ObjectHandle p_Handle, Type p_ComponentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ObjectHandle GameObject_FindWithTag(string p_Tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ObjectHandle GameObject_FindWithUUID(ulong p_UUID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool GameObject_IsValid(ObjectHandle p_Handle);
        #endregion

        #region SceneManager

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int SceneManager_GetConfigLoadMode();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SceneManager_IsActive(ulong p_SceneHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SceneManager_IsImported(ulong p_SceneHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SceneManager_SetConfigLoadMode(int p_LoadMode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static SceneHandle SceneManager_ImportScene(string p_ScenePath);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static SceneHandle SceneManager_ImportSceneAsync(string p_ScenePath);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static SceneHandle SceneManager_LoadSceneByPath(string p_ScenePath, int p_Mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static SceneHandle SceneManager_LoadScene(ulong p_SceneHandle, int p_Mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static SceneHandle SceneManager_LoadSceneAsyncByPath(string p_ScenePath, int p_Mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static SceneHandle SceneManager_LoadSceneAsync(ulong p_SceneHandle, int p_Mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SceneManager_UnloadSceneByPath(string p_ScenePath);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SceneManager_UnloadScene(ulong p_SceneHandle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SceneManager_Pause(bool p_Value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SceneManager_IsPaused();

        #endregion

        #region Scene

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Scene_Pause(ulong p_Handle, bool p_Value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Scene_IsPaused(ulong p_Handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Scene_IsUUIDValid(ulong p_Handle, ulong p_UUID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Scene_FindWithTag(ulong p_Handle, string p_Tag);

        #endregion

        #region TagComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string TagComponent_GetTag(ObjectHandle p_Handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TagComponent_SetTag(ObjectHandle p_Handle, string p_Tag);
        #endregion

        #region TransformComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetLocalTranslation(ObjectHandle p_Handle, out Vector3 p_Result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetLocalTranslation(ObjectHandle p_Handle, ref Vector3 p_Value);
        #endregion

        #region RuntimeComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool RuntimeComponent_IsEnabled(ObjectHandle p_Handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool RuntimeComponent_IsActive(ObjectHandle p_Handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RuntimeComponent_SetEnabled(ObjectHandle p_Handle, bool p_Value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RuntimeComponent_SetActive(ObjectHandle p_Handle, bool p_Value);

        #endregion

        #region TextRendererComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string TextRendererComponent_GetString(ObjectHandle p_Handle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetString(ObjectHandle p_Handle, string p_String);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetFont(ObjectHandle p_Handle, string p_Path);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string TextRendererComponent_GetFontPath(ObjectHandle p_Handle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string TextRendererComponent_GetFontName(ObjectHandle p_Handle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_GetColor(ObjectHandle p_Handle, out Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetColor(ObjectHandle p_Handle, ref Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_GetBgColor(ObjectHandle p_Handle, out Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetBgColor(ObjectHandle p_Handle, ref Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_GetCharBgColor(ObjectHandle p_Handle, out Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetCharBgColor(ObjectHandle p_Handle, ref Vector4 p_Color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool TextRendererComponent_GetDrawBg(ObjectHandle p_Handle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetDrawBg(ObjectHandle p_Handle, bool p_Enable);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float TextRendererComponent_GetKerning(ObjectHandle p_Handle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetKerning(ObjectHandle p_Handle, float p_Kerning);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float TextRendererComponent_GetLineSpacing(ObjectHandle p_Handle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TextRendererComponent_SetLineSpacing(ObjectHandle p_Handle, float p_LineSpacing);
        #endregion

        #region Input
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyPressed(KeyCode p_Keycode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyJustReleased(KeyCode p_Keycode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyJustPressed(KeyCode p_Keycode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyJustHeld(KeyCode p_Keycode);


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

        #region Box2D
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void B2_GetLinearVelocity(ObjectHandle p_Handle, out Vector2 p_OutVelocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void B2_SetLinearVelocity(ObjectHandle p_Handle, ref Vector2 p_Velocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float B2_GetAngularVelocity(ObjectHandle p_Handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void B2_SetAngularVelocity(ObjectHandle p_Handle, float p_AngVelocity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void B2_ApplyForce(ObjectHandle p_Handle, ref Vector2 p_Force);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void B2_ApplyLinearImpulse(ObjectHandle p_Handle, ref Vector2 p_Impulse);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void B2_ApplyAngularImpulse(ObjectHandle p_Handle, float p_Impulse);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void B2_ApplyTorque(ObjectHandle p_Handle, float p_Torque);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void B2_GetGravity(ObjectHandle p_Handle, out Vector2 p_OutGravity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void B2_GetRealGravity(ObjectHandle p_Handle, out Vector2 p_OutGravity);

        #endregion

        #region CharacterBody2DComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterBody2DComponent_MoveAndSlide(ObjectHandle p_Handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CharacterBody2DComponent_MoveAndCollide(ObjectHandle p_Handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool CharacterBody2DComponent_IsOnFloor(ObjectHandle p_Handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool CharacterBody2DComponent_IsOnWall(ObjectHandle p_Handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool CharacterBody2DComponent_IsOnCeiling(ObjectHandle p_Handle);

        #endregion

    }
} // namespace KTN