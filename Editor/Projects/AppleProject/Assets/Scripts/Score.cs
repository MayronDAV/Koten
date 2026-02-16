using KTN;
using System;
using System.Numerics;


namespace Apple
{	
	public class Score : ScriptBehavior
	{
		private Apple.Player m_Player;
        private TextRendererComponent m_TextRenderer;

		void OnCreate()
		{
			var obj = GameObject.FindWithTag("Player");
            if (obj == null || !obj.IsValid())
            {
                Console.WriteLine("Failed to find Player object!");
            }

            m_Player = GetScriptBehavior<Apple.Player>(obj);

            m_TextRenderer = GetComponent<TextRendererComponent>();
            if (m_TextRenderer == null)
            {
                Console.WriteLine("Failed to find TextRendererComponent!");
            }
		}

		void OnUpdate()
        {
            if (m_Player != null && m_TextRenderer != null)
            {
                float score = m_Player.GetScore();
                m_TextRenderer.String = $"Score: {score}";
            }
        }

	}
} // namespace Apple