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
        public static bool IsPaused { get => InternalCalls.SceneManager_IsPaused(); set => InternalCalls.SceneManager_Pause(value); }

        public static Scene GetScene(ulong p_SceneHandle)
        {
            var scene = new Scene(p_SceneHandle);
            return scene;
        }

        public static bool IsImported(ulong p_SceneHandle)
        {
            return InternalCalls.SceneManager_IsImported(p_SceneHandle);
        }

        public static bool IsActive(ulong p_SceneHandle)
        {
            return InternalCalls.SceneManager_IsActive(p_SceneHandle);
        }

        public static Scene ImportScene(string p_ScenePath)
        {
            var sceneHandle = InternalCalls.SceneManager_ImportScene(p_ScenePath);
            var scene = new Scene(sceneHandle.Handle);
            return scene;
        }

        public static Scene LoadScene(ulong p_SceneHandle)
        {
            var sceneHandle = InternalCalls.SceneManager_LoadScene(p_SceneHandle, (int)Mode);
            var scene = new Scene(sceneHandle.Handle);
            return scene;
        }

        public static Scene LoadScene(ulong p_SceneHandle, LoadMode p_Mode)
		{
			var sceneHandle = InternalCalls.SceneManager_LoadScene(p_SceneHandle, (int)p_Mode);
            var scene = new Scene(sceneHandle.Handle);
            return scene;
        }

        public static Scene LoadScene(string p_ScenePath)
        {
            var sceneHandle = InternalCalls.SceneManager_LoadSceneByPath(p_ScenePath, (int)Mode);
            var scene = new Scene(sceneHandle.Handle);
            return scene;
        }

        public static Scene LoadScene(string p_ScenePath, LoadMode p_Mode)
        {
            var sceneHandle = InternalCalls.SceneManager_LoadSceneByPath(p_ScenePath, (int)p_Mode);
            var scene = new Scene(sceneHandle.Handle);
            return scene;
        }

        public static Scene LoadSceneAsync(ulong p_SceneHandle)
        {
            var sceneHandle = InternalCalls.SceneManager_LoadSceneAsync(p_SceneHandle, (int)Mode);
            var scene = new Scene(sceneHandle.Handle);
            return scene;
        }

        public static Scene LoadSceneAsync(ulong p_SceneHandle, LoadMode p_Mode)
        {
            var sceneHandle = InternalCalls.SceneManager_LoadSceneAsync(p_SceneHandle, (int)p_Mode);
            var scene = new Scene(sceneHandle.Handle);
            return scene;
        }

        public static Scene LoadSceneAsync(string p_ScenePath)
        {
            var sceneHandle = InternalCalls.SceneManager_LoadSceneAsyncByPath(p_ScenePath, (int)Mode);
            var scene = new Scene(sceneHandle.Handle);
            return scene;
        }

        public static Scene LoadSceneAsync(string p_ScenePath, LoadMode p_Mode)
        {
            var sceneHandle = InternalCalls.SceneManager_LoadSceneAsyncByPath(p_ScenePath, (int)p_Mode);
            var scene = new Scene(sceneHandle.Handle);
            return scene;
        }

        public static void UnloadScene(ulong p_SceneHandle)
        {
            InternalCalls.SceneManager_UnloadScene(p_SceneHandle);
        }

        public static void UnloadScene(string p_ScenePath)
        {
            InternalCalls.SceneManager_UnloadSceneByPath(p_ScenePath);
        }
    };

} // namespace KTN