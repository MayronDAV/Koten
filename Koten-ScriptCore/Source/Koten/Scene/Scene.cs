using System;

namespace KTN
{
    public class Scene
    {
        internal Scene() { }
        internal Scene(ulong p_Handle)
        {
            Handle = p_Handle;
        }

        public ulong Handle { get; internal set; }
        public bool IsActive { get => InternalCalls.SceneManager_IsActive(Handle); internal set {} }
        public bool IsImported { get => InternalCalls.SceneManager_IsImported(Handle); internal set {} }
        public bool IsPaused { get => InternalCalls.Scene_IsPaused(Handle); set => InternalCalls.Scene_Pause(Handle, value); }

        public bool IsUUIDValid(ulong p_ID)
        {
            return InternalCalls.Scene_IsUUIDValid(Handle, p_ID);
        }

        public GameObject FindWithTag(string p_Tag)
        {
            var handle = InternalCalls.Scene_FindWithTag(Handle, p_Tag);
            if (handle == 0)
                return null;

            return new GameObject(handle, Handle);
        }

        public void Load(LoadMode p_Mode = LoadMode.Single)
        {
            InternalCalls.SceneManager_LoadScene(Handle, (int)p_Mode);
        }

        public void LoadAsync(LoadMode p_Mode = LoadMode.Single)
        {
            InternalCalls.SceneManager_LoadSceneAsync(Handle, (int)p_Mode);
        }

        public void Unload()
        {
            InternalCalls.SceneManager_UnloadScene(Handle);
        }
    }


} // namespace KTN
