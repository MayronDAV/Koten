using System;
using System.Collections.Generic;

namespace KTN
{
	public enum LoadMode
	{
		Single = 0,
		Additive
    };

    public class SceneManager
	{
        public static LoadMode Mode { get => (LoadMode)InternalCalls.SceneManager_GetConfigLoadMode(); set => InternalCalls.SceneManager_SetConfigLoadMode((int)value); }

        public static bool LoadScene(ulong p_SceneHandle)
        {
            return InternalCalls.SceneManager_LoadScene(p_SceneHandle, (int)Mode);
        }

        public static bool LoadScene(ulong p_SceneHandle, LoadMode p_Mode)
		{
			return InternalCalls.SceneManager_LoadScene(p_SceneHandle, (int)p_Mode);
		}

        public static bool LoadScene(string p_ScenePath)
        {
            return InternalCalls.SceneManager_LoadSceneByPath(p_ScenePath, (int)Mode);
        }

        public static bool LoadScene(string p_ScenePath, LoadMode p_Mode)
        {
            return InternalCalls.SceneManager_LoadSceneByPath(p_ScenePath, (int)p_Mode);
        }

        public static void LoadSceneAsync(ulong p_SceneHandle)
        {
            InternalCalls.SceneManager_LoadSceneAsync(p_SceneHandle, (int)Mode);
        }

        public static void LoadSceneAsync(ulong p_SceneHandle, LoadMode p_Mode)
        {
            InternalCalls.SceneManager_LoadSceneAsync(p_SceneHandle, (int)p_Mode);
        }

        public static void LoadSceneAsync(string p_ScenePath)
        {
            InternalCalls.SceneManager_LoadSceneAsyncByPath(p_ScenePath, (int)Mode);
        }

        public static void LoadSceneAsync(string p_ScenePath, LoadMode p_Mode)
        {
            InternalCalls.SceneManager_LoadSceneAsyncByPath(p_ScenePath, (int)p_Mode);
        }

        public static void UnloadScene(ulong p_SceneHandle)
        {
            InternalCalls.SceneManager_UnloadScene(p_SceneHandle);
        }

        public static void UnloadScene(string p_ScenePath)
        {
            InternalCalls.SceneManager_UnloadSceneByPath(p_ScenePath);
        }

        public static void Pause(bool p_Value)
        {
            InternalCalls.SceneManager_Pause(p_Value);
        }

        public static bool IsPaused()
		{
			return InternalCalls.SceneManager_IsPaused();
        }
    };

} // namespace KTN