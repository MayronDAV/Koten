using System;
using System.Runtime.CompilerServices;

namespace KTN
{
	public class Entity
	{
		protected Entity() { ID = 0; SceneHandle = 0; }

		internal Entity(ulong p_ID, ulong p_SceneHandle)
		{
			ID = p_ID;
			SceneHandle = p_SceneHandle;
        }

		public readonly ulong ID;
        public readonly ulong SceneHandle;

        public Vector3 LocalTranslation
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalTranslation(ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetLocalTranslation(ID, ref value);
            }
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

			T component = new T() { Entity = this };
			return component;
		}

		public Entity GetEntityByTag(string p_Tag)
		{
			var handle = InternalCalls.GameObject_FindWithTag(p_Tag);
			if (handle.ID == 0 || handle.SceneHandle == 0)
				return null;

			return new Entity(handle.ID, handle.SceneHandle);
		}

		public Entity GetEntityByUUID(ulong p_UUID)
		{
            var handle = InternalCalls.GameObject_FindWithUUID(p_UUID);
            if (handle.ID == 0 || handle.SceneHandle == 0)
                return null;

            return new Entity(handle.ID, handle.SceneHandle);
        }

        public T As<T>() where T : Entity, new()
		{
			object instance = InternalCalls.GetScriptInstance(ID);
			return instance as T;
		}
	}
} // namespace KTN