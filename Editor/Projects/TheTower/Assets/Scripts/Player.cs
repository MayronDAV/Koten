using KTN;
using System;

namespace TW
{
    public class Player : ScriptBehavior
    {
        public float Speed = 5.0f;
        private CharacterBody2DComponent m_Component;

        void OnCreate()
        {
            m_Component = GetComponent<CharacterBody2DComponent>();
        }

        void OnUpdate()
        {
            float speed = Speed;
            Vector2 velocity = Vector2.Zero;

            if (Input.IsControllerConnected(0))
            {
                velocity.X = Input.GetControllerAxis(0, GamepadCode.AxisLeftX);
                velocity.Y = -Input.GetControllerAxis(0, GamepadCode.AxisLeftY);
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
            if (m_Component != null)
            {
                m_Component.SetLinearVelocity(velocity);
                m_Component.MoveAndSlide();	
            }
        }
    }
}