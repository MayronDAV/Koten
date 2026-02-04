using KTN;
using System;
using System.Numerics;


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

		private Asset m_PlayerAsset;

		private float m_OldTime = 0.0f;
		private float m_CurTime = 0.0f;

		void OnCreate()
		{
			m_Component = GetComponent<CharacterBody2DComponent>();

			m_PlayerAsset = AssetManager.FindWithPath("Prefabs\\Player.ktprefab");
		}

		void OnTriggerEnter(ulong p_Sensor)
		{
			Console.WriteLine($"Player.OnTriggerBegin: {p_Sensor}");

			if (p_Sensor == 15841598909273239145)
			{
				SceneManager.LoadSceneAsync("Scenes\\Example.ktscn"); 
			}
		}

		void OnCollisionEnter(ulong p_Entity)
		{
			Console.WriteLine($"Player.OnCollisionEnter: {p_Entity}");

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

		void OnUpdate()
		{
			Timestep += Time.DeltaTime;
			m_DeltaTime = Time.DeltaTime;

			m_CurTime = Time.CurTime;
			if (m_CurTime - m_OldTime >= 1.5f)
			{
				m_OldTime = m_CurTime;
				var rand = new Random();

				var count = rand.Next(4);
				for (int i = 0; i < count; i++)
				{
					Vector3 spawnerPos = Vector3.Zero;
					float min = -5.0f;
					float max = 5.0f;
					spawnerPos.X = (float)rand.NextDouble() * (max - min) + min;

					Instantiate(m_PlayerAsset, spawnerPos, Vector3.Zero);
				}
			}


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