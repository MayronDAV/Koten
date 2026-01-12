using System;

namespace KTN
{
    public class GameObject : Object
    {
        internal GameObject() { base.ID = 0; base.SceneHandle = 0; }

        internal GameObject(ulong p_ID, ulong p_SceneHandle)
        {
            ID = p_ID;
            SceneHandle = p_SceneHandle;
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.GameObject_HasComponent(new ObjectHandle(ID, SceneHandle), componentType);
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { ID = ID, SceneHandle = SceneHandle };
            return component;
        }

        static public GameObject FindWithTag(string p_Tag)
        {
            var handle = InternalCalls.GameObject_FindWithTag(p_Tag);
            if (handle.ID == 0 || handle.SceneHandle == 0)
                return null;

            return new GameObject(handle.ID, handle.SceneHandle);
        }

        static public GameObject FindWithUUID(ulong p_UUID)
        {
            var handle = InternalCalls.GameObject_FindWithUUID(p_UUID);
            if (handle.ID == 0 || handle.SceneHandle == 0)
                return null;

            return new GameObject(handle.ID, handle.SceneHandle);
        }
    }
} // namespace KTN