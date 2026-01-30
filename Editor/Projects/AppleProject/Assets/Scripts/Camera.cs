using KTN;


namespace Apple
{
	public class Camera : ScriptBehavior
	{
		private Transform m_Player;
		private Transform m_Local;

		public float DistanceFromPlayer = 5.0f;

		void OnCreate()
		{
			var gameObject = GameObject.FindWithTag("Player");
			m_Player = gameObject.GetComponent<Transform>();
			m_Local = GetComponent<Transform>();
		}

		void OnUpdate()
		{
			Vector3 playerTranslation = m_Player.LocalTranslation;
			playerTranslation.Z = DistanceFromPlayer;
			m_Local.LocalTranslation = playerTranslation;
		}
	}
} // namespace Apple