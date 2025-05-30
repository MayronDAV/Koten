using System;

namespace KTN
{
	[AttributeUsage(AttributeTargets.Field)]
	public class ShowInEditor : Attribute {}

	[AttributeUsage(AttributeTargets.Field)]
	public class SerializeField : Attribute { }

} // namespace KTN