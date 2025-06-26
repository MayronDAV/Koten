using KTN;
using System;


namespace Sandbox
{
	public class Text : Entity
	{
		private String m_Text = "Hello, KTN!";
        private TextRendererComponent m_TextRenderer;
        private int m_Cooldown = 0;
        private static int s_CurIndex = 0;

		void OnCreate()
		{
            m_TextRenderer = GetComponent<TextRendererComponent>();
            m_TextRenderer.String = String.Empty;
		}

		void OnUpdate()
		{
            if (m_Cooldown >= 100)
            {
                if (s_CurIndex >= m_Text.Length)
                {
                    s_CurIndex = 0;
                    m_TextRenderer.String = String.Empty; // Reset the text when done
                }

                m_TextRenderer.String += m_Text[s_CurIndex++];
                m_Cooldown = 0;
            }

            m_Cooldown++;
		}

	}
} // namespace Sandbox