using System;

namespace KTN
{
    public enum AssetType
    {
        None = 0,
        Scene,
        Font,
        Texture2D,
        PhysicsMaterial2D,
        Prefab,
        Material,
        TextureAtlas,
        Animation,
        AnimationController,
        ShaderStage,
        Shader
    };

    public class Asset : Object
    {
        internal Asset() { base.ID = 0; base.SceneHandle = 0; base.Type = 0; }

        public Asset(ulong p_ID, ulong p_SceneHandle, AssetType p_Type)
        {
            ID = p_ID;
            SceneHandle = p_SceneHandle;
            Type = (Int32)p_Type;
        }

        public Asset(Object p_Object)
            : base(p_Object) { }

        public bool IsValid() => ID != 0 && InternalCalls.AssetManager_IsAssetHandleValid(ID);
        public bool IsLoaded() => InternalCalls.AssetManager_IsAssetLoaded(ID);
    }
}
