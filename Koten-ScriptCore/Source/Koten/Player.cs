using KTN;
using System;


namespace Sandbox
{
	public class Player : Entity
	{
		private Transform m_Transform;

		void OnCreate()
		{
			Console.WriteLine($"Player.OnCreate - {ID}");

			m_Transform = GetComponent<Transform>();
		}

		void OnUpdate()
		{
			// Console.WriteLine($"Player.OnUpdate: {ts}");

			float speed = 5.0f;
			Vector3 velocity = Vector3.Zero;

			if (Input.IsKeyPressed(KeyCode.W))
				velocity.Y = 1.0f;
			else if (Input.IsKeyPressed(KeyCode.S))
				velocity.Y = -1.0f;

			if (Input.IsKeyPressed(KeyCode.A))
				velocity.X = -1.0f;
			else if (Input.IsKeyPressed(KeyCode.D))
				velocity.X = 1.0f;

			velocity *= speed;

			Vector3 translation = m_Transform.LocalTranslation;
			translation += velocity * Time.DeltaTime;
			m_Transform.LocalTranslation = translation;
		}

	}
} // namespace Sandbox