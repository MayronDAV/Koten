using System;

namespace KTN
{
    public abstract class Component : Object
    {
        protected Component()
            : base()
        {
        }
    }

    public class Transform : Component
    {
        public Vector3 LocalTranslation
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalTranslation(ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetLocalTranslation(ID, ref value);
            }
        }
    }

    public class IDComponent : Component
    {
    }

    public class TagComponent : Component
    {
        public string Tag
        {
            get => InternalCalls.TagComponent_GetTag(ID);
            set => InternalCalls.TagComponent_SetTag(ID, value);
        }
    }

    public class  RuntimeComponent : Component
    {
        public bool Enabled
        {
            get => InternalCalls.RuntimeComponent_IsEnabled(ID);
            set => InternalCalls.RuntimeComponent_SetEnabled(ID, value);
        }

        public bool Active
        {
            get => InternalCalls.RuntimeComponent_IsActive(ID);
            set => InternalCalls.RuntimeComponent_SetActive(ID, value);
        }
    }

    public class SpriteComponent : Component
    {
    }

    public class LineRendererComponent : Component
    {
    }

    public class TextRendererComponent : Component
    {
        public string String
        {
            get => InternalCalls.TextRendererComponent_GetString(ID);
            set => InternalCalls.TextRendererComponent_SetString(ID, value);
        }

        public string Font
        {
            get => InternalCalls.TextRendererComponent_GetFontPath(ID);
            set => InternalCalls.TextRendererComponent_SetFont(ID, value);
        }

        public Vector4 Color
        {
            get
            {
                InternalCalls.TextRendererComponent_GetColor(ID, out Vector4 color);
                return color;
            }
            set => InternalCalls.TextRendererComponent_SetColor(ID, ref value);
        }

        public Vector4 BgColor
        {
            get
            {
                InternalCalls.TextRendererComponent_GetBgColor(ID, out Vector4 color);
                return color;
            }
            set => InternalCalls.TextRendererComponent_SetBgColor(ID, ref value);
        }

        public Vector4 CharBgColor
        {
            get
            {
                InternalCalls.TextRendererComponent_GetCharBgColor(ID, out Vector4 color);
                return color;
            }
            set => InternalCalls.TextRendererComponent_SetCharBgColor(ID, ref value);
        }

        public bool DrawBg
        {
            get => InternalCalls.TextRendererComponent_GetDrawBg(ID);
            set => InternalCalls.TextRendererComponent_SetDrawBg(ID, value);
        }

        public float Kerning
        {
            get => InternalCalls.TextRendererComponent_GetKerning(ID);
            set => InternalCalls.TextRendererComponent_SetKerning(ID, value);
        }

        public float LineSpacing
        {
            get => InternalCalls.TextRendererComponent_GetLineSpacing(ID);
            set => InternalCalls.TextRendererComponent_SetLineSpacing(ID, value);
        }

    }

    public class CameraComponent : Component
    {
    }

    public class HierarchyComponent : Component
    {
    }

    public class PhysicsBody : Component
    {
        public Vector2 Gravity
        {
            get
            {
                InternalCalls.B2_GetGravity(ID, out Vector2 gravity);
                return gravity;
            }
        }

        public Vector2 RealGravity
        {
            get
            {
                InternalCalls.B2_GetRealGravity(ID, out Vector2 gravity);
                return gravity;
            }
        }

        public Vector2 GetLinearVelocity()
        {
           InternalCalls.B2_GetLinearVelocity(ID, out Vector2 result);
           return result;
        }

        public void SetLinearVelocity(Vector2 p_Velocity)
        {
            InternalCalls.B2_SetLinearVelocity(ID, ref p_Velocity);
        }

        public float GetAngularVelocity()
        {        
            return InternalCalls.B2_GetAngularVelocity(ID);
        }

        public void SetAngularVelocity(float p_Velocity)
        {
            InternalCalls.B2_SetAngularVelocity(ID, p_Velocity);
        }

        public void ApplyForce(Vector2 p_Force)
        {
            InternalCalls.B2_ApplyForce(ID, ref p_Force);
        }

        public void ApplyLinearImpulse(Vector2 p_Impulse)
        {
            InternalCalls.B2_ApplyLinearImpulse(ID, ref p_Impulse);
        }

        public void ApplyAngularImpulse(float p_Impulse)
        {
            InternalCalls.B2_ApplyAngularImpulse(ID, p_Impulse);
        }

        public void ApplyTorque(float p_Torque)
        {
            InternalCalls.B2_ApplyTorque(ID, p_Torque);
        }
    }

    public class Rigidbody2DComponent : PhysicsBody
    {     
	}

    public class CharacterBody2DComponent : PhysicsBody
    {
        public bool OnFloor
        {
            get => InternalCalls.CharacterBody2DComponent_IsOnFloor(ID);
        }
        public bool OnWall
        {
            get => InternalCalls.CharacterBody2DComponent_IsOnWall(ID);
        }
        public bool OnCeiling
        {
            get => InternalCalls.CharacterBody2DComponent_IsOnCeiling(ID);
        }

        public void MoveAndSlide()
        {
            InternalCalls.CharacterBody2DComponent_MoveAndSlide(ID);
        }

        public void MoveAndCollide()
        {
            InternalCalls.CharacterBody2DComponent_MoveAndCollide(ID);
        }
    }

    public class StaticBody2DComponent : PhysicsBody
    {
    }

    public class BodyShape2DComponent : Component
    {
    }

    public class ScriptComponent : Component
    {
    }

    public class PrefabComponent : Component
    {
    }

} // namespace KTN