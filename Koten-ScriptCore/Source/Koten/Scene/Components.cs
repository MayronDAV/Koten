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

} // namespace KTN