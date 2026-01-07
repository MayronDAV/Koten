using KTN;


namespace Sandbox
{
	public class Camera : Entity
	{
		private Entity m_Player;

		public float DistanceFromPlayer = 5.0f;

		[ShowInEditor]
		private bool m_FollowPlayer = false;

		void OnCreate()
		{
			m_Player = GetEntityByTag("Player");
		}

		void OnUpdate()
		{
			if (Input.IsControllerConnected(0))
			{
				if (Input.IsControllerButtonReleased(0, GamepadCode.ButtonSouth))
					m_FollowPlayer = !m_FollowPlayer;
			}
			else
			{
				if (Input.IsKeyJustReleased(KeyCode.P)) 
					m_FollowPlayer = !m_FollowPlayer;
			}

			if (m_FollowPlayer)
			{
				Vector3 playerTranslation = m_Player.LocalTranslation;
				playerTranslation.Z = DistanceFromPlayer;
				LocalTranslation = playerTranslation;
				return;
			}

			float speed = 5.0f;
			Vector3 velocity = Vector3.Zero;

			if (Input.IsControllerConnected(0))
			{
				velocity.X = Input.GetControllerAxis(0, GamepadCode.AxisRightX);
				velocity.Y = Input.GetControllerAxis(0, GamepadCode.AxisRightY) * -1.0f;
			}
			else
			{
				if (Input.IsKeyPressed(KeyCode.Up))
					velocity.Y = 1.0f;
				else if (Input.IsKeyPressed(KeyCode.Down))
					velocity.Y = -1.0f;

				if (Input.IsKeyPressed(KeyCode.Left))
					velocity.X = -1.0f;
				else if (Input.IsKeyPressed(KeyCode.Right))
					velocity.X = 1.0f;
			}

			velocity *= speed;

			Vector3 translation = LocalTranslation;
			translation += velocity * Time.DeltaTime;
			translation.Z = DistanceFromPlayer;
			LocalTranslation = translation;
		}

	}
} // namespace Sandbox