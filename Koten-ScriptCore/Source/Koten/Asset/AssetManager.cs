using System;

namespace KTN
{
    static public class AssetManager
    {
        static public Asset FindWithPath(string p_Path)
        {
            var handle = InternalCalls.AssetManager_FindWithPath(p_Path);
            var obj = new Asset
            {
                ID = handle.ID,
                SceneHandle = handle.SceneHandle,
                Type = handle.Type
            };
            return obj;
        }

        static public bool IsAssetHandleValid(ulong p_Handle) => p_Handle != 0 && InternalCalls.AssetManager_IsAssetHandleValid(p_Handle);
        static public bool IsAssetLoaded(ulong p_Handle) => InternalCalls.AssetManager_IsAssetLoaded(p_Handle);
    }
}
