using System;

namespace KTN
{
    public class ScriptBehavior : RuntimeComponent
    {
        protected ScriptBehavior()
            : base()
        {
        }

        internal ScriptBehavior(ulong p_ID, ulong p_SceneHandle)
        {
            ID = p_ID;
            SceneHandle = p_SceneHandle;
            GameObject = new GameObject(ID, SceneHandle);
        }

        public GameObject GameObject { get; internal set; }

        public bool HasComponent<T>() where T : Component, new()
        {
            return GameObject.HasComponent<T>();
        }

        public T GetComponent<T>() where T : Component, new()
        {
            return GameObject.GetComponent<T>();
        }

        public Object Instantiate(Object p_Object, Vector3 p_Position, Vector3 p_Rotation)
        {
            var obj = new Object(p_Object.ID, SceneHandle) { Type = p_Object.Type };

            var inst = Object.InstantiateObject(obj, p_Position, p_Rotation);
            return new Object(inst.ID, inst.SceneHandle, obj.Type);
        }

        public T Instantiate<T>(Object p_Object, Vector3 p_Position, Vector3 p_Rotation) where T : Component, new()
        {
            var obj = new Object(p_Object.ID, SceneHandle) { Type = p_Object.Type };

            var inst = Object.InstantiateObject(obj, p_Position, p_Rotation);
            return new T() { ID = inst.ID, SceneHandle = inst.SceneHandle, Type = obj.Type };
        }
    }

} // namespace KTN
