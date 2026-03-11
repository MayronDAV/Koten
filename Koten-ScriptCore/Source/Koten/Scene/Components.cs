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
                InternalCalls.TransformComponent_GetLocalTranslation(GetHandle(), out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetLocalTranslation(GetHandle(), ref value);
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
            get => InternalCalls.TagComponent_GetTag(GetHandle());
            set => InternalCalls.TagComponent_SetTag(GetHandle(), value);
        }
    }

    public class  RuntimeComponent : Component
    {
        public bool Enabled
        {
            get => InternalCalls.RuntimeComponent_IsEnabled(GetHandle());
            set => InternalCalls.RuntimeComponent_SetEnabled(GetHandle(), value);
        }

        public bool Active
        {
            get => InternalCalls.RuntimeComponent_IsActive(GetHandle());
            set => InternalCalls.RuntimeComponent_SetActive(GetHandle(), value);
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
            get => InternalCalls.TextRendererComponent_GetString(GetHandle());
            set => InternalCalls.TextRendererComponent_SetString(GetHandle(), value);
        }

        public string Font
        {
            get => InternalCalls.TextRendererComponent_GetFontPath(GetHandle());
            set => InternalCalls.TextRendererComponent_SetFont(GetHandle(), value);
        }

        public Vector4 Color
        {
            get
            {
                InternalCalls.TextRendererComponent_GetColor(GetHandle(), out Vector4 color);
                return color;
            }
            set => InternalCalls.TextRendererComponent_SetColor(GetHandle(), ref value);
        }

        public Vector4 BgColor
        {
            get
            {
                InternalCalls.TextRendererComponent_GetBgColor(GetHandle(), out Vector4 color);
                return color;
            }
            set => InternalCalls.TextRendererComponent_SetBgColor(GetHandle(), ref value);
        }

        public Vector4 CharBgColor
        {
            get
            {
                InternalCalls.TextRendererComponent_GetCharBgColor(GetHandle(), out Vector4 color);
                return color;
            }
            set => InternalCalls.TextRendererComponent_SetCharBgColor(GetHandle(), ref value);
        }

        public bool DrawBg
        {
            get => InternalCalls.TextRendererComponent_GetDrawBg(GetHandle());
            set => InternalCalls.TextRendererComponent_SetDrawBg(GetHandle(), value);
        }

        public float Kerning
        {
            get => InternalCalls.TextRendererComponent_GetKerning(GetHandle());
            set => InternalCalls.TextRendererComponent_SetKerning(GetHandle(), value);
        }

        public float LineSpacing
        {
            get => InternalCalls.TextRendererComponent_GetLineSpacing(GetHandle());
            set => InternalCalls.TextRendererComponent_SetLineSpacing(GetHandle(), value);
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
                InternalCalls.B2_GetGravity(GetHandle(), out Vector2 gravity);
                return gravity;
            }
        }

        public Vector2 RealGravity
        {
            get
            {
                InternalCalls.B2_GetRealGravity(GetHandle(), out Vector2 gravity);
                return gravity;
            }
        }

        public Vector2 GetLinearVelocity()
        {
           InternalCalls.B2_GetLinearVelocity(GetHandle(), out Vector2 result);
           return result;
        }

        public void SetLinearVelocity(Vector2 p_Velocity)
        {
            InternalCalls.B2_SetLinearVelocity(GetHandle(), ref p_Velocity);
        }

        public float GetAngularVelocity()
        {        
            return InternalCalls.B2_GetAngularVelocity(GetHandle());
        }

        public void SetAngularVelocity(float p_Velocity)
        {
            InternalCalls.B2_SetAngularVelocity(GetHandle(), p_Velocity);
        }

        public void ApplyForce(Vector2 p_Force)
        {
            InternalCalls.B2_ApplyForce(GetHandle(), ref p_Force);
        }

        public void ApplyLinearImpulse(Vector2 p_Impulse)
        {
            InternalCalls.B2_ApplyLinearImpulse(GetHandle(), ref p_Impulse);
        }

        public void ApplyAngularImpulse(float p_Impulse)
        {
            InternalCalls.B2_ApplyAngularImpulse(GetHandle(), p_Impulse);
        }

        public void ApplyTorque(float p_Torque)
        {
            InternalCalls.B2_ApplyTorque(GetHandle(), p_Torque);
        }
    }

    public class Rigidbody2DComponent : PhysicsBody
    {     
    }

    public class CharacterBody2DComponent : PhysicsBody
    {
        public bool OnFloor
        {
            get => InternalCalls.CharacterBody2DComponent_IsOnFloor(GetHandle());
        }
        public bool OnWall
        {
            get => InternalCalls.CharacterBody2DComponent_IsOnWall(GetHandle());
        }
        public bool OnCeiling
        {
            get => InternalCalls.CharacterBody2DComponent_IsOnCeiling(GetHandle());
        }

        public void MoveAndSlide()
        {
            InternalCalls.CharacterBody2DComponent_MoveAndSlide(GetHandle());
        }

        public void MoveAndCollide()
        {
            InternalCalls.CharacterBody2DComponent_MoveAndCollide(GetHandle());
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