using KTN;


namespace Sandbox
{
	public class Camera : Entity
	{
		void OnUpdate()
		{
			float speed = 5.0f;
			Vector3 velocity = Vector3.Zero;

			if (Input.IsControllerConnected(0))
			{
				velocity.X = Input.GetControllerAxis(0, GamepadCode.AxisRightX);
				velocity.Y = Input.GetControllerAxis(0, GamepadCode.AxisRightY) * -1.0f;
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
			}

			velocity *= speed;

			Vector3 translation = LocalTranslation;
			translation += velocity * Time.DeltaTime;
			LocalTranslation = translation;
		}

	}
} // namespace Sandbox