using System;



namespace KTN
{
    public static class Window
    {
        public static void SetTitle(string p_Title)
        {
            InternalCalls.Window_SetTitle(p_Title);
        }

        public static bool IsVsync
        { 
            get => InternalCalls.Window_IsVsync(); 
            set => InternalCalls.Window_SetVsync(value);
        }

        public static bool IsMinimized => InternalCalls.Window_IsMinimized();
        public static bool IsMaximized=> InternalCalls.Window_IsMaximized();

        public static UInt32 Width
        { 
            get => InternalCalls.Window_GetWidth();
            set => InternalCalls.Window_SetWidth(value);
        }

        public static UInt32 Height
        { 
            get => InternalCalls.Window_GetHeight();
            set => InternalCalls.Window_SetHeight(value);
        }

        public static void Resize(UInt32 p_Width, UInt32 p_Height)
        {
            InternalCalls.Window_Resize(p_Width, p_Height);
        }
    }
}
