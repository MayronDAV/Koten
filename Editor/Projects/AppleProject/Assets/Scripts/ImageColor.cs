using KTN;
using System;


namespace Apple
{
	public class ImageColor : ScriptBehavior
	{
		private UIImageComponent m_Component;

        private float m_OldTime = 0.0f;
        private float m_CurTime = 0.0f;

		void OnCreate()
		{
			m_Component = GetComponent<UIImageComponent>();
            if (m_Component == null)
            {
                Console.WriteLine("ImageColor: UIImageComponent not found!");
            }
		}

		void OnUpdate()
		{
            if (m_Component == null)
                return;

            m_CurTime = Time.CurTime;
            if (m_CurTime - m_OldTime >= 1.0f)
            {
                m_OldTime = m_CurTime;
                var rand          = new Random();
                rand.Next(3);

                float min         = 0.0f;
                float max         = 1.0f;
                Vector4 color     = m_Component.Color;
                color.X           = (float)rand.NextDouble() * (max - min) + min;
                color.Y           = (float)rand.NextDouble() * (max - min) + min;
                color.Z           = (float)rand.NextDouble() * (max - min) + min;

                m_Component.Color = color;
            }
		}
	}
} // namespace Apple