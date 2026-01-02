using System;
using System.Runtime.CompilerServices;

namespace KTN
{
	public class Entity
	{
		protected Entity() { ID = 0; }

		internal Entity(ulong id)
		{
			ID = id;
		}

		public readonly ulong ID;

		public Vector3 LocalTranslation
		{
			get
			{
				InternalCalls.TransformComponent_GetLocalTranslation(ID, out Vector3 result);
				return result;
			}
			set
			{
				InternalCalls.TransformComponent_SetLocalTranslation(ID, ref value);
			}
		}

		public Vector2 Velocity
		{
			get
			{
				InternalCalls.B2_GetLinearVelocity(ID, out Vector2 result);
				return result;
			}
			set
			{
				InternalCalls.B2_SetLinearVelocity(ID, ref value);
            }
        }

		public bool OnFloor
		{
			get
			{
				return InternalCalls.CharacterBody2DComponent_IsOnFloor(ID);
			}
        }

        public bool OnWall
        {
            get
            {
                return InternalCalls.CharacterBody2DComponent_IsOnWall(ID);
            }
        }

        public bool OnCeiling
        {
            get
            {
                return InternalCalls.CharacterBody2DComponent_IsOnCeiling(ID);
            }
        }

        public void MoveAndSlide()
		{
			InternalCalls.CharacterBody2DComponent_MoveAndSlide(ID);
		}

        public void MoveAndCollide()
        {
            InternalCalls.CharacterBody2DComponent_MoveAndCollide(ID);
        }

        public bool HasComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);
			return InternalCalls.Entity_HasComponent(ID, componentType);
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
			ulong entityID = InternalCalls.Entity_GetEntityByTag(p_Tag);
			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		public Entity GetEntityByUUID(ulong p_UUID)
		{
			if (InternalCalls.Entity_IsValid(p_UUID))
				return new Entity(p_UUID);

			return null;
        }

        public T As<T>() where T : Entity, new()
		{
			object instance = InternalCalls.GetScriptInstance(ID);
			return instance as T;
		}
	}
} // namespace KTN