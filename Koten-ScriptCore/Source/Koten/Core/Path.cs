namespace KTN.IO
{
    public class Path
    {
        private string m_Path;

        public Path(string p_Path = "")
        {
            m_Path = p_Path;
        }

        public Path(char[] p_Path)
        {
            m_Path = new string(p_Path);
        }

        public void Rename(string p_NewName)
        {
            if (Empty())
                return;

            m_Path = Parent() / p_NewName;
            m_Path.Normalize();

            if (Exists())
                FileSystem.Rename(m_Path, p_NewName);
        }

        public void Normalize()
        {
            if (Empty())
                return;

            m_Path = FileSystem.Normalize(m_Path);
        }

        public void ReplaceExtension(string p_NewExtension)
        {
            if (Empty())
                return;

            m_Path = FileSystem.ReplaceExtension(m_Path, p_NewExtension);
        }

        public void CopyTo(Path p_Destination)
        {
            if (Empty() || p_Destination.Empty())
                return;

            FileSystem.Copy(m_Path, p_Destination);
        }

        public void MoveTo(Path p_Destination)
        {
            if (Empty() || p_Destination.Empty())
                return;

            if (FileSystem.Move(m_Path, p_Destination))
                m_Path = p_Destination;
        }

        public bool CreateDirectories()
        {
            if (Empty())
                return false;

            return FileSystem.CreateDirectories(m_Path);
        }

        public bool Remove()
        {
            if (Empty() || !Exists())
                return false;

            return FileSystem.Remove(m_Path);
        }

        public string Name()                    { return FileSystem.GetName(m_Path); }
        public string Stem()                    { return FileSystem.GetStem(m_Path); }
        public string Extension()               { return FileSystem.GetExtension(m_Path); }
        public Path Parent()                    { return FileSystem.GetParent(m_Path); }
        public Path Absolute()                  { return FileSystem.GetAbsolute(m_Path); }
        public Path Relative(Path p_RelativeTo) { return FileSystem.GetRelative(m_Path, p_RelativeTo.IsDirectory() ? p_RelativeTo : p_RelativeTo.Parent()); }

        public bool Empty()         { return string.IsNullOrEmpty(m_Path); }
        public bool Exists()        { return !Empty() && FileSystem.Exists(m_Path); }
        public bool IsAbsolute()    { return !Empty() && FileSystem.IsAbsolute(m_Path); }
        public bool IsRelative()    { return !Empty() && FileSystem.IsRelative(m_Path); }
        public bool IsRegularFile() { return !Empty() && FileSystem.IsRegularFile(m_Path); }
        public bool IsDirectory()   { return !Empty() && FileSystem.IsDirectory(m_Path); }
        public string String()      { return m_Path; }

        public static implicit operator Path(string p_Path)
        {
            return new Path(p_Path);
        }

        public static implicit operator Path(char[] p_Path)
        {
            return new Path(p_Path);
        }

        public static implicit operator string(Path p_Path)
        {
            return p_Path.String();
        }

        public static Path operator /(Path p_Left, Path p_Right)
        {
            var path = new Path(p_Left.String() + "/" + p_Right.String());
            path.Normalize();
            return path;
        }

        public static Path operator +(Path p_Left, Path p_Right)
        {
            var path = new Path(p_Left.String() + "/" + p_Right.String());
            path.Normalize();
            return path;
        }

        public static implicit operator bool(Path p_Path)
        {
            return !p_Path.Empty() && p_Path.Exists();
        }
    }

} // namespace KTN.IO
