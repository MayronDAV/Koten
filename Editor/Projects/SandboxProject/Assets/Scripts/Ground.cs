using KTN;
using System;


namespace Sandbox
{	
	public class Ground : ScriptBehavior
	{
		void OnCollisionEnter(ulong p_Entity)
		{
			Console.WriteLine($"Ground.OnCollisionEnter: {p_Entity}");

			var gameObject = FindWithUUID(p_Entity);
			if (gameObject == null || !gameObject.IsValid())
			{
				Console.WriteLine("Failed to find game object!");
				return;
			}

			var tc = gameObject.GetComponent<TagComponent>();
			var tag = tc.Tag;

			if (tag == "Player")
				Destroy(gameObject);
		}
	}
} // namespace Sandbox