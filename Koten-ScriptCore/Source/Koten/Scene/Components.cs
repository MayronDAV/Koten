using System;

namespace KTN
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }

    public class Transform : Component
    {
        public Vector3 LocalTranslation
        {
            get
            {
                InternalCalls.TransformComponent_GetLocalTranslation(Entity.ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetLocalTranslation(Entity.ID, ref value);
            }
        }
    }

    public class IDComponent : Component
    {
        public ulong ID => Entity.ID;
    }

    public class TagComponent : Component
    {
        public string Tag
        {
            get => InternalCalls.TagComponent_GetTag(Entity.ID);
            set => InternalCalls.TagComponent_SetTag(Entity.ID, value);
        }
    }

    public class  RuntimeComponent : Component
    {
        public bool Enabled
        {
            get => InternalCalls.RuntimeComponent_IsEnabled(Entity.ID);
            set => InternalCalls.RuntimeComponent_SetEnabled(Entity.ID, value);
        }

        public bool Active
        {
            get => InternalCalls.RuntimeComponent_IsActive(Entity.ID);
            set => InternalCalls.RuntimeComponent_SetActive(Entity.ID, value);
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
            get => InternalCalls.TextRendererComponent_GetString(Entity.ID);
            set => InternalCalls.TextRendererComponent_SetString(Entity.ID, value);
        }

        public string Font
        {
            get => InternalCalls.TextRendererComponent_GetFontPath(Entity.ID);
            set => InternalCalls.TextRendererComponent_SetFont(Entity.ID, value);
        }

        public Vector4 Color
        {
            get
            {
                InternalCalls.TextRendererComponent_GetColor(Entity.ID, out Vector4 color);
                return color;
            }
            set => InternalCalls.TextRendererComponent_SetColor(Entity.ID, ref value);
        }

        public Vector4 BgColor
        {
            get
            {
                InternalCalls.TextRendererComponent_GetBgColor(Entity.ID, out Vector4 color);
                return color;
            }
            set => InternalCalls.TextRendererComponent_SetBgColor(Entity.ID, ref value);
        }

        public Vector4 CharBgColor
        {
            get
            {
                InternalCalls.TextRendererComponent_GetCharBgColor(Entity.ID, out Vector4 color);
                return color;
            }
            set => InternalCalls.TextRendererComponent_SetCharBgColor(Entity.ID, ref value);
        }

        public bool DrawBg
        {
            get => InternalCalls.TextRendererComponent_GetDrawBg(Entity.ID);
            set => InternalCalls.TextRendererComponent_SetDrawBg(Entity.ID, value);
        }

        public float Kerning
        {
            get => InternalCalls.TextRendererComponent_GetKerning(Entity.ID);
            set => InternalCalls.TextRendererComponent_SetKerning(Entity.ID, value);
        }

        public float LineSpacing
        {
            get => InternalCalls.TextRendererComponent_GetLineSpacing(Entity.ID);
            set => InternalCalls.TextRendererComponent_SetLineSpacing(Entity.ID, value);
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
                InternalCalls.B2_GetGravity(Entity.ID, out Vector2 gravity);
                return gravity;
            }
        }

        public Vector2 RealGravity
        {
            get
            {
                InternalCalls.B2_GetRealGravity(Entity.ID, out Vector2 gravity);
                return gravity;
            }
        }

        public Vector2 GetLinearVelocity()
        {
           InternalCalls.B2_GetLinearVelocity(Entity.ID, out Vector2 result);
           return result;
        }

        public void SetLinearVelocity(Vector2 p_Velocity)
        {
            InternalCalls.B2_SetLinearVelocity(Entity.ID, ref p_Velocity);
        }

        public float GetAngularVelocity()
        {        
            return InternalCalls.B2_GetAngularVelocity(Entity.ID);
        }

        public void SetAngularVelocity(float p_Velocity)
        {
            InternalCalls.B2_SetAngularVelocity(Entity.ID, p_Velocity);
        }

        public void ApplyForce(Vector2 p_Force)
        {
            InternalCalls.B2_ApplyForce(Entity.ID, ref p_Force);
        }

        public void ApplyLinearImpulse(Vector2 p_Impulse)
        {
            InternalCalls.B2_ApplyLinearImpulse(Entity.ID, ref p_Impulse);
        }

        public void ApplyAngularImpulse(float p_Impulse)
        {
            InternalCalls.B2_ApplyAngularImpulse(Entity.ID, p_Impulse);
        }

        public void ApplyTorque(float p_Torque)
        {
            InternalCalls.B2_ApplyTorque(Entity.ID, p_Torque);
        }
    }

    public class Rigidbody2DComponent : PhysicsBody
    {     
	}

    public class CharacterBody2DComponent : PhysicsBody
    {
        public bool OnFloor
        {
            get => InternalCalls.CharacterBody2DComponent_IsOnFloor(Entity.ID);
        }
        public bool OnWall
        {
            get => InternalCalls.CharacterBody2DComponent_IsOnWall(Entity.ID);
        }
        public bool OnCeiling
        {
            get => InternalCalls.CharacterBody2DComponent_IsOnCeiling(Entity.ID);
        }

        public void MoveAndSlide()
        {
            InternalCalls.CharacterBody2DComponent_MoveAndSlide(Entity.ID);
        }

        public void MoveAndCollide()
        {
            InternalCalls.CharacterBody2DComponent_MoveAndCollide(Entity.ID);
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