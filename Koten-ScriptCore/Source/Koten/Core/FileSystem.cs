namespace KTN.IO
{
    public static class FileSystem
    {
        public static bool Exists(string p_Path)
        {
            if (string.IsNullOrEmpty(p_Path))
                return false;

            return InternalCalls.FileSystem_Exists(p_Path);
        }

        public static bool Exists(Path p_Path)
        {
            return Exists(p_Path.String());
        }

        public static bool WriteFile(Path p_Path, string p_Text)
        {
            if (p_Path.Empty())
                return false;

            return InternalCalls.FileSystem_WriteFile(p_Path.String(), p_Text);
        }

        public static bool WriteFileBinary(Path p_Path, byte[] p_Data)
        {
            if (p_Path.Empty())
                return false;

            return InternalCalls.FileSystem_WriteFileBinary(p_Path.String(), p_Data);
        }

        public static string ReadFile(Path p_Path)
        {
            if (p_Path.Empty() || !p_Path.Exists())
                return string.Empty;

            if (!p_Path.IsRegularFile())
                return string.Empty;

            return InternalCalls.FileSystem_ReadFile(p_Path.String());
        }

        public static byte[] ReadFileBinary(Path p_Path)
        {
            if (p_Path.Empty() || !p_Path.Exists())
                return new byte[0];

            if (!p_Path.IsRegularFile())
                return new byte[0];

            return InternalCalls.FileSystem_ReadFileBinary(p_Path.String());
        }

        public static bool CreateDirectories(Path p_Path)
        {
            if (p_Path.Empty())
                return false;

            return InternalCalls.FileSystem_CreateDirectories(p_Path.String());
        }

        public static bool Copy(Path p_Source, Path p_Destination)
        {
            if (p_Source.Empty() || p_Destination.Empty() || !p_Source.Exists())
                return false;

            if (!p_Destination.Exists())
                CreateDirectories(p_Destination);

            return InternalCalls.FileSystem_Copy(p_Source.String(), p_Destination.String());
        }

        public static bool Move(Path p_Source, Path p_Destination)
        {
            if (p_Source.Empty() || p_Destination.Empty() || !p_Source.Exists())
                return false;

            if (!p_Destination.Exists())
                CreateDirectories(p_Destination);

            return InternalCalls.FileSystem_Move(p_Source.String(), p_Destination.String());
        }

        public static bool Remove(Path p_Path)
        {
            if (p_Path.Empty() || !p_Path.Exists())
                return false;

            return InternalCalls.FileSystem_Remove(p_Path.String());
        }

        public static void Rename(Path p_Path, string p_NewName)
        {
            if (p_Path.Empty() || !p_Path.Exists())
                return;

            InternalCalls.FileSystem_Rename(p_Path.String(), p_NewName);
        }

        public static string GetName(Path p_Path)
        {
            if (p_Path.Empty())
                return string.Empty;

            return InternalCalls.FileSystem_GetName(p_Path.String());
        }

        public static string GetStem(Path p_Path)
        {
            if (p_Path.Empty())
                return string.Empty;

            return InternalCalls.FileSystem_GetStem(p_Path.String());
        }

        public static string GetExtension(Path p_Path)
        {
            if (p_Path.Empty())
                return string.Empty;

            return InternalCalls.FileSystem_GetExtension(p_Path.String());
        }

        public static Path GetParent(Path p_Path)
        {
            if (p_Path.Empty())
                return new Path();

            return InternalCalls.FileSystem_GetParent(p_Path.String());
        }

        public static Path GetAbsolute(Path p_Path)
        {
            if (p_Path.Empty())
                return new Path();

            return InternalCalls.FileSystem_GetAbsolute(p_Path.String());
        }

        public static Path GetRelative(Path p_Path, Path p_RelativeTo)
        {
            if (p_Path.Empty() || p_RelativeTo.Empty())
                return new Path();

            return InternalCalls.FileSystem_GetRelative(p_Path.String(), p_RelativeTo.String());
        }

        public static Path ReplaceExtension(Path p_Path, string p_NewExtension)
        {
            if (p_Path.Empty())
                return new Path();

            return InternalCalls.FileSystem_ReplaceExtension(p_Path.String(), p_NewExtension);
        }

        public static Path Normalize(Path p_Path)
        {
            if (p_Path.Empty())
                return new Path();

            return InternalCalls.FileSystem_Normalize(p_Path.String());
        }

        public static bool IsAbsolute(Path p_Path)
        {
            if (p_Path.Empty())
                return false;

            return InternalCalls.FileSystem_IsAbsolute(p_Path.String());
        }

        public static bool IsRelative(Path p_Path)
        {
            if (p_Path.Empty())
                return false;

            return InternalCalls.FileSystem_IsRelative(p_Path.String());
        }

        public static bool IsRegularFile(Path p_Path)
        {
            if (p_Path.Empty() || !p_Path.Exists())
                return false;

            return InternalCalls.FileSystem_IsRegularFile(p_Path.String());
        }

        public static bool IsDirectory(Path p_Path)
        {
            if (p_Path.Empty() || !p_Path.Exists())
                return false;

            return InternalCalls.FileSystem_IsDirectory(p_Path.String());
        }
    }

} // namespace KTN.IO
