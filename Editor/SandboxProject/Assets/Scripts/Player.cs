using KTN;
using System;


namespace Sandbox
{	
	public class Player : ScriptBehavior
	{
		public float Speed = 5.0f;
		public float Timestep = 0.0f;

		[SerializeField]
		[ShowInEditor]
		private float m_DeltaTime = 0.0f;

		private const float m_JumpVelocity = 400;

		private CharacterBody2DComponent m_Component;

		void OnCreate()
		{
			m_Component = GetComponent<CharacterBody2DComponent>();

			var box = GameObject.FindWithTag("Box");

			Vector3 pos = Vector3.Zero;
			Instantiate(box, pos, Vector3.Zero);
			pos.X += 10.0f;
			Instantiate(box, pos, Vector3.Zero);
			pos.X += 10.0f;
			Instantiate(box, pos, Vector3.Zero);

			var circle = AssetManager.FindWithPath("Prefabs\\Circle.ktprefab");
			pos.X = 10.0f;
			pos.Y = 10.0f;
			Instantiate(circle, pos, Vector3.Zero);

			var player = AssetManager.FindWithPath("Prefabs\\Player.ktprefab");
			pos.X = -5.0f;
			pos.Y = 10.0f;
			Instantiate(player, pos, Vector3.Zero);
		}

		void OnTriggerEnter(ulong p_Sensor)
		{
			Console.WriteLine($"Player.OnTriggerBegin: {p_Sensor}");

			if (p_Sensor == 15841598909273239145)
			{
				SceneManager.LoadSceneAsync("Scenes\\Example.ktscn"); 
			}
		}

		void OnCollisionEnter(ulong p_Collider)
		{
			Console.WriteLine($"Player.OnCollisionEnter: {p_Collider}");
		}

		void OnUpdate()
		{
			Timestep += Time.DeltaTime;
			m_DeltaTime = Time.DeltaTime;

			//Console.WriteLine($"Player.OnUpdate: {m_DeltaTime}");

			float speed = Speed;
			Vector2 velocity = Vector2.Zero;

			// if (!m_Component.OnFloor)
			// 	velocity += m_Component.Gravity * 300.0f * m_DeltaTime;

			if (Input.IsControllerConnected(0))
			{
				velocity.X = Input.GetControllerAxis(0, GamepadCode.AxisLeftX);
				velocity.Y = Input.GetControllerAxis(0, GamepadCode.AxisLeftY) * -1.0f;

				//if (m_Component.OnFloor)
				//	velocity *= speed;

				// if (Input.IsControllerButtonReleased(0, GamepadCode.ButtonSouth))
				// {
				// 	velocity.Y = m_JumpVelocity;
				// }
			}
			else
			{
				if (Input.IsKeyPressed(KeyCode.W))
					velocity.Y = 1.0f;
				else if (Input.IsKeyPressed(KeyCode.S))
					velocity.Y = -1.0f;

				if (Input.IsKeyPressed(KeyCode.A))
					velocity.X = -1.0f;
				else if (Input.IsKeyPressed(KeyCode.D))
					velocity.X = 1.0f;

				//if (m_Component.OnFloor)
				//	velocity *= speed;

				// if (Input.IsKeyJustReleased(KeyCode.Space))
				// {
				// 	velocity.Y = m_JumpVelocity;
				// }
			}

			velocity *= speed;

			m_Component.SetLinearVelocity(velocity);
			m_Component.MoveAndCollide();
		}

	}
} // namespace Sandbox