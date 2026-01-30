using KTN;
using System;


namespace Apple
{	
	public class Spawner : ScriptBehavior
	{
		private Asset m_Apple;
		private Asset m_Can;
		private Transform m_Local;

		public bool m_GameStarted = false;

		private float m_OldTime = 0.0f;
		private float m_CurTime = 0.0f;

		void OnCreate()
		{
			m_Apple = AssetManager.FindWithPath("Prefabs\\Apple.ktprefab");
			m_Can = AssetManager.FindWithPath("Prefabs\\Can.ktprefab");

			m_Local = GetComponent<Transform>();
		}

		void OnUpdate()
		{
			m_CurTime = Time.CurTime;
			if (!m_GameStarted)
			{
				if (m_CurTime - m_OldTime >= 5.0f)
				{
					m_GameStarted = true;
					m_CurTime = 0.0f;
					m_OldTime = m_CurTime;
				}
			}
			else if (m_CurTime - m_OldTime >= 1.5f)
			{
				m_OldTime = m_CurTime;
				var rand = new Random();

				var count = rand.Next(4);
				for (int i = 0; i < count; i++)
				{
					Vector3 spawnerPos = m_Local.LocalTranslation;
					float min = -5.0f;
					float max = 5.0f;
					spawnerPos.X = (float)rand.NextDouble() * (max - min) + min;

					int type = rand.Next();
					if (type % 2 == 0)
					{
						Instantiate(m_Apple, spawnerPos, Vector3.Zero);
					}
					else
					{
						Instantiate(m_Can, spawnerPos, Vector3.Zero);
					}
				}

			}
		}

	}
} // namespace Apple