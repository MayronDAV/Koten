using System;

namespace KTN
{
    public class Object
    {
        internal Object()
        {
            ID = 0;
            SceneHandle = 0;
        }

        internal Object(ulong p_ID, ulong p_SceneHandle)
        {
            ID = p_ID;
            SceneHandle = p_SceneHandle;
        }

        internal Object(ulong p_ID, ulong p_SceneHandle, Int32 p_Type)
        {
            ID = p_ID;
            SceneHandle = p_SceneHandle;
            Type = p_Type;
        }

        public ulong ID { get; internal set; }
        public ulong SceneHandle { get; internal set; }
        internal Int32 Type = 0;

        static internal Object InstantiateObject(Object p_Object, Vector3 p_Position, Vector3 p_Rotation)
        {
            var handle = InternalCalls.Object_Instantiate(new ObjectHandle { ID = p_Object.ID, SceneHandle = p_Object.SceneHandle, Type = p_Object.Type }, ref p_Position, ref p_Rotation);
            var obj = new Object
            {
                ID = handle.ID,
                SceneHandle = handle.SceneHandle,
                Type = handle.Type,
            };
            return obj;
        }

        public T As<T>() where T : Object, new()
        {
            if (!typeof(T).IsSubclassOf(typeof(Object)))
                return null;

            var obj = new T
            {
                ID = ID,
                SceneHandle = SceneHandle,
                Type = Type
            };
            return obj;
        }
    }
}
