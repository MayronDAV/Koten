namespace KTN
{
	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class Transform : Component
	{
		public Vector3 LocalTranslation
		{
			get
			{
				InternalCalls.TransformComponent_GetLocalTranslation(Entity.ID, out Vector3 translation);
				return translation;
			}
			set
			{
				InternalCalls.TransformComponent_SetLocalTranslation(Entity.ID, ref value);
			}
		}
	}

} // namespace KTN