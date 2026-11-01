using KTN;
using System;

namespace TW
{
    public class Play : ScriptBehavior
    {
        private UIInputComponent m_Component;
        [ShowInEditor]
        private bool m_Loading = false;

        void OnCreate()
        {
            m_Component = GetComponent<UIInputComponent>();
        }

        void OnUpdate()
        {
            if (m_Component == null)
                return;

            if (m_Component.Pressed && !m_Loading)
            {
                SceneManager.LoadScene("Scenes/Loading.ktscn", LoadMode.Single);
                SceneManager.LoadSceneAsync("Scenes/Floor.ktscn", LoadMode.Single);
                m_Loading = true;
            }
        }
    }

    public class PlayFile : ScriptBehavior
    {
        private UIInputComponent m_Component;
        [ShowInEditor]
        private bool m_Loading = false;

        void OnCreate()
        {
            m_Component = GetComponent<UIInputComponent>();
        }

        void OnUpdate()
        {
            if (m_Component == null)
                return;

            if (m_Component.Pressed && !m_Loading)
            {
                SceneManager.LoadScene("Scenes/Loading.ktscn", LoadMode.Single);
                SceneManager.LoadSceneAsync("Scenes/FloorFile.ktscn", LoadMode.Single);
                m_Loading = true;
            }
        }
    }

} // namespace TW