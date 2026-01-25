using KTN;
using System;


namespace Apple
{	
	public class Player : ScriptBehavior
	{
		public float Speed = 5.0f;

		private CharacterBody2DComponent m_Component;

		void OnCreate()
		{
			m_Component = GetComponent<CharacterBody2DComponent>();
		}

		void OnCollisionEnter(ulong p_Entity)
		{
			Console.WriteLine($"Player.OnCollisionEnter: {p_Entity}");

			var gameObject = GameObject.FindWithUUID(p_Entity);
			if (!gameObject.IsValid())
			{
				Console.WriteLine("Failed to find game object!");
				return;
			}

			var tc = gameObject.GetComponent<TagComponent>();
			var tag = tc.Tag;
			if (tag == "Apple" || tag == "Can")
				Destroy(gameObject);
		}

		void OnUpdate()
		{
			float speed = Speed;
			Vector2 velocity = Vector2.Zero;

			if (Input.IsControllerConnected(0))
			{
				velocity.X = Input.GetControllerAxis(0, GamepadCode.AxisLeftX);
			}
			else
			{
				if (Input.IsKeyPressed(KeyCode.A))
					velocity.X = -1.0f;
				else if (Input.IsKeyPressed(KeyCode.D))
					velocity.X = 1.0f;
			}

			velocity *= speed;

			m_Component.SetLinearVelocity(velocity);
			m_Component.MoveAndSlide();
		}

	}
} // namespace Apple