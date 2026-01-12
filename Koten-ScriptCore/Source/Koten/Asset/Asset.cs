using System;

namespace KTN
{
    enum AssetType
    {
		Scene = 1,
		Font,
		Texture2D,
		PhysicsMaterial2D,
		Prefab
    };

    public class Asset : Object
    {
        internal Asset() { base.ID = 0; base.SceneHandle = 0; base.Type = 0; }

        internal Asset(ulong p_ID, ulong p_SceneHandle, AssetType p_Type)
        {
            ID = p_ID;
            SceneHandle = p_SceneHandle;
            Type = (Int32)p_Type;
        }

        public bool IsValid() => ID != 0 && InternalCalls.AssetManager_IsAssetHandleValid(ID);
        public bool IsLoaded() => InternalCalls.AssetManager_IsAssetLoaded(ID);
    }
}
