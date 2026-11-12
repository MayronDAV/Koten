using System;
using System.Text;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;


namespace KTN.IO
{
    public class FileEmitter
    {
        private readonly StringBuilder m_Builder;
        private readonly string m_IndentString;

        private int m_CurrIndent = 0;

        public FileEmitter()
            : this("  ")
        {
        }

        public FileEmitter(string p_IndentString)
        {
            m_Builder      = new StringBuilder();
            m_IndentString = p_IndentString ?? throw new ArgumentNullException(nameof(p_IndentString));
        }

        public int IndentLevel => m_CurrIndent;

        public string Data()
        {
            return m_Builder.ToString();
        }

        public void Write(string p_Key, string p_Value)
        {
            ValidateKey(p_Key);

            if (p_Value == null)
                throw new ArgumentNullException(nameof(p_Value));

            if (p_Value.IndexOf('\n') < 0 && p_Value.IndexOf('\r') < 0)
            {
                WriteLine(p_Key + ": " + p_Value);
                return;
            }

            string normalized = p_Value.Replace("\r\n", "\n").Replace('\r', '\n');
            normalized        = normalized.TrimEnd('\n');

            string[] lines    = normalized.Split(new[] { '\n' }, StringSplitOptions.None);
            Write(p_Key, lines);
        }

        public void Write(string p_Key, string[] p_Values)
        {
            ValidateKey(p_Key);

            if (p_Values == null)
                throw new ArgumentNullException(nameof(p_Values));

            WriteLine(p_Key + ": [");

            m_CurrIndent++;
            foreach (string value in p_Values)
            {
                if (value == null)
                    throw new ArgumentException("Array values cannot contain null.", nameof(p_Values));

                if (value.IndexOf('\n') >= 0 || value.IndexOf('\r') >= 0)
                {
                    throw new ArgumentException("Each array item must contain only one line.", nameof(p_Values));
                }

                WriteLine(value);
            }

            m_CurrIndent--;
            WriteLine("]");
        }

        public void BeginBlock(string p_Key)
        {
            ValidateKey(p_Key);

            WriteLine(p_Key + ":");
            m_CurrIndent++;
        }

        public bool EndBlock()
        {
            if (m_CurrIndent <= 0)
                return false;

            m_CurrIndent--;
            return true;
        }

        private void WriteLine(string p_Text)
        {
            WriteText(p_Text);
            m_Builder.Append(Environment.NewLine);
        }

        private void WriteText(string p_Text)
        {
            if (string.IsNullOrEmpty(p_Text))
                return;

            bool atLineStart = true;

            foreach (char ch in p_Text)
            {
                if (ch == '\n' || ch == '\r')
                {
                    m_Builder.Append(ch);
                    if (ch == '\n')
                        atLineStart = true;
                    continue;
                }

                if (atLineStart)
                {
                    AppendIndent();
                    atLineStart = false;
                }

                m_Builder.Append(ch);
            }
        }

        public void Clear()
        {
            m_Builder.Clear();
            m_CurrIndent = 0;
        }

        public bool Save(Path p_Path)
        {
            if (p_Path.Empty())
                return false;

            return FileSystem.WriteFile(p_Path, Data());
        }

        private void ValidateKey(string p_Key)
        {
            if (string.IsNullOrWhiteSpace(p_Key))
                throw new ArgumentException("Key cannot be null, empty or whitespace.", nameof(p_Key));

            if (p_Key.IndexOf(':') >= 0  || p_Key.IndexOf('\n') >= 0 ||
                p_Key.IndexOf('\r') >= 0 || p_Key.IndexOf('[') >= 0  || p_Key.IndexOf(']') >= 0)
            {
                throw new ArgumentException("Key cannot contain ':', brackets or line breaks.", nameof(p_Key));
            }
        }

        private void AppendIndent()
        {
            for (int i = 0; i < m_CurrIndent; i++)
                m_Builder.Append(m_IndentString);
        }
    }

    public class FileNode
    {
        private readonly Dictionary<string, object> m_Values = new Dictionary<string, object>();

        public string Name { get; private set; }

        public IEnumerable<string> Keys => m_Values.Keys;

        public int Count => m_Values.Count;

        internal FileNode(string p_Name)
        {
            Name = p_Name;
        }

        public bool ContainsKey(string p_Key)
        {
            return m_Values.ContainsKey(p_Key);
        }

        public object Get(string p_Key)
        {
            object value;
            return m_Values.TryGetValue(p_Key, out value) ? value : null;
        }

        public string GetString(string p_Key)
        {
            return Get(p_Key) as string;
        }

        public string[] GetStringArray(string p_Key)
        {
            return Get(p_Key) as string[];
        }

        public FileNode GetNode(string p_Key)
        {
            return Get(p_Key) as FileNode;
        }

        public bool TryGetString(string p_Key, out string p_Value)
        {
            p_Value = GetString(p_Key);
            return p_Value != null;
        }

        public bool TryGetStringArray(string p_Key, out string[] p_Values)
        {
            p_Values = GetStringArray(p_Key);
            return p_Values != null;
        }

        public bool TryGetNode(string p_Key, out FileNode p_Node)
        {
            p_Node = GetNode(p_Key);
            return p_Node != null;
        }

        public bool TryGetValue(string p_Key, out object p_Value)
        {
            return m_Values.TryGetValue(p_Key, out p_Value);
        }

        internal bool Add(string p_Key, object p_Value)
        {
            if (string.IsNullOrEmpty(p_Key))
                return false;

            if (!(p_Value is string) && !(p_Value is string[]) && !(p_Value is FileNode))
                return false;

            if (m_Values.ContainsKey(p_Key))
                return false;

            m_Values.Add(p_Key, p_Value);
            return true;
        }
    }

    public static class FileReader
    {
        public static FileNode Read(string p_Text)
        {
            if (p_Text == null)
                throw new ArgumentNullException(nameof(p_Text));

            string[] lines = p_Text.Split(new[] { "\r\n", "\n", "\r" }, StringSplitOptions.None);

            string indentString = DetectIndentString(lines);

            FileNode root = new FileNode(string.Empty);

            var nodes = new List<FileNode>();
            nodes.Add(root);

            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i];

                if (string.IsNullOrWhiteSpace(line))
                    continue;

                int indent = GetIndentLevel(line, indentString, i + 1);

                string content = line.Substring(indent * indentString.Length);

                int separator = content.IndexOf(':');

                if (separator < 0)
                {
                    throw new FormatException("Missing ':' at line " + (i + 1) + ". Content: [" + content + "]");
                }

                if (separator == 0)
                {
                    throw new FormatException("Empty key at line " + (i + 1) + ". Content: [" + content + "]");
                }

                string key = content.Substring(0, separator).Trim();
                string value = content.Substring(separator + 1);

                if (value.StartsWith(" "))
                    value = value.Substring(1);

                if (key.Length == 0)
                {
                    throw new FormatException("Empty key at line " + (i + 1) + ".");
                }

                if (indent >= nodes.Count)
                {
                    throw new FormatException("Unexpected indentation at line " + (i + 1) + ".");
                }

                if (nodes.Count > indent + 1)
                {
                    nodes.RemoveRange(indent + 1,nodes.Count - indent - 1);
                }

                FileNode parent = nodes[indent];

                if (value == "[")
                {
                    var values  = new List<string>();
                    bool closed = false;

                    while (++i < lines.Length)
                    {
                        string itemLine    = lines[i];

                        int itemIndent     = GetIndentLevel(itemLine, indentString, i + 1);
                        string itemContent = itemLine.Substring(itemIndent * indentString.Length);
                        if (itemIndent == indent && itemContent == "]")
                        {
                            closed = true;
                            break;
                        }

                        if (itemIndent != indent + 1)
                        {
                            throw new FormatException("Invalid array indentation at line " + (i + 1) + ".");
                        }
                        values.Add(itemContent);
                    }

                    if (!closed)
                    {
                        throw new FormatException("Unclosed array for key '" + key + "'.");
                    }

                    if (!parent.Add(key, values.ToArray()))
                    {
                        throw new FormatException("Duplicate key '" + key + "'.");
                    }

                    continue;
                }

                if (value.Length == 0)
                {
                    FileNode child = new FileNode(key);
                    if (!parent.Add(key, child))
                    {
                        throw new FormatException("Duplicate key '" + key + "' at line " + (i + 1) + ".");
                    }

                    nodes.Add(child);
                }
                else
                {
                    if (!parent.Add(key, value))
                    {
                        throw new FormatException("Duplicate key '" + key + "' at line " + (i + 1) + ".");
                    }
                }
            }

            return root;
        }

        public static FileNode ReadFile(Path p_Path)
        {
            if (p_Path.Empty() || !p_Path.Exists())
                return null;

            string content = FileSystem.ReadFile(p_Path);
            return Read(content);
        }

        private static string DetectIndentString(string[] p_Lines)
        {
            bool foundSpaces = false;
            bool foundTabs   = false;
            int spaceGcd     = 0;

            foreach (string line in p_Lines)
            {
                if (string.IsNullOrWhiteSpace(line))
                    continue;

                int position = 0;

                while (position < line.Length && (line[position] == ' ' || line[position] == '\t'))
                {
                    if (line[position] == ' ')
                        foundSpaces = true;
                    else
                        foundTabs   = true;

                    position++;
                }

                if (foundSpaces && foundTabs)
                {
                    throw new FormatException("Mixed spaces and tabs in indentation.");
                }

                if (foundTabs)
                {
                    for (int i = 0; i < position; i++)
                    {
                        if (line[i] != '\t')
                        {
                            throw new FormatException("Inconsistent indentation style.");
                        }
                    }
                }
                else if (position > 0)
                {
                    spaceGcd = GreatestCommonDivisor(spaceGcd, position);
                }
            }

            if (foundTabs)
                return "\t";

            if (spaceGcd > 0)
                return new string(' ', spaceGcd);

            return "  ";
        }

        private static int GetIndentLevel(string p_Line, string p_IndentString, int p_LineNumber)
        {
            int position = 0;
            int level    = 0;

            while (position < p_Line.Length && (p_Line[position] == ' ' || p_Line[position] == '\t'))
            {
                if (position + p_IndentString.Length > p_Line.Length ||
                    string.Compare(p_Line, position, p_IndentString, 0, p_IndentString.Length, StringComparison.Ordinal) != 0)
                {
                    throw new FormatException("Invalid indentation at line " + p_LineNumber + ".");
                }

                position += p_IndentString.Length;
                level++;
            }

            return level;
        }

        private static int GreatestCommonDivisor(int p_A, int p_B)
        {
            while (p_B != 0)
            {
                int remainder = p_A % p_B;
                p_A = p_B;
                p_B = remainder;
            }

            return p_A;
        }
    }

    public sealed class BinaryFileWriter : IDisposable
    {
        private readonly MemoryStream m_Stream;
        private readonly System.IO.BinaryWriter m_Writer;
        private bool m_Disposed;

        public long Length
        {
            get { return m_Stream.Length; }
        }

        public byte[] Data
        {
            get
            {
                EnsureNotDisposed();
                m_Writer.Flush();
                return m_Stream.ToArray();
            }
        }

        public BinaryFileWriter()
        {
            m_Stream = new MemoryStream();
            m_Writer = new System.IO.BinaryWriter(m_Stream);
        }

        public void Write(byte p_Value) { m_Writer.Write(p_Value); }
        public void Write(sbyte p_Value) { m_Writer.Write(p_Value); }
        public void Write(short p_Value) { m_Writer.Write(p_Value); }
        public void Write(ushort p_Value) { m_Writer.Write(p_Value); }
        public void Write(int p_Value) { m_Writer.Write(p_Value); }
        public void Write(uint p_Value) { m_Writer.Write(p_Value); }
        public void Write(long p_Value) { m_Writer.Write(p_Value); }
        public void Write(ulong p_Value) { m_Writer.Write(p_Value); }
        public void Write(float p_Value) { m_Writer.Write(p_Value); }
        public void Write(double p_Value) { m_Writer.Write(p_Value); }
        public void Write(bool p_Value) { m_Writer.Write(p_Value); }
        public void Write(char p_Value) { m_Writer.Write(p_Value); }
        public void Write(string p_Value) { m_Writer.Write(p_Value); }
        public void Write(Path p_Value) { m_Writer.Write(p_Value.String()); }
        public void Write(decimal p_Value) { m_Writer.Write(p_Value); }
        public void Write(DateTime p_Value) { m_Writer.Write(p_Value.ToBinary()); }
        public void Write(Guid p_Value) { m_Writer.Write(p_Value.ToByteArray()); }
        public void Write(byte[] p_Data) { WriteBytes(p_Data); }

        public void WriteBytes(byte[] p_Data)
        {
            if (p_Data == null)
                throw new ArgumentNullException(nameof(p_Data));

            m_Writer.Write(p_Data.Length);
            m_Writer.Write(p_Data);
        }

        public void WriteRawBytes(byte[] p_Data)
        {
            if (p_Data == null)
                throw new ArgumentNullException(nameof(p_Data));

            m_Writer.Write(p_Data);
        }

        public void Write(int[] p_Values)
        {
            if (p_Values == null)
                throw new ArgumentNullException(nameof(p_Values));

            m_Writer.Write(p_Values.Length);

            foreach (int value in p_Values)
                m_Writer.Write(value);
        }

        public void Write(float[] p_Values)
        {
            if (p_Values == null)
                throw new ArgumentNullException(nameof(p_Values));

            m_Writer.Write(p_Values.Length);

            foreach (float value in p_Values)
                m_Writer.Write(value);
        }

        public void Write(double[] p_Values)
        {
            if (p_Values == null)
                throw new ArgumentNullException(nameof(p_Values));

            m_Writer.Write(p_Values.Length);

            foreach (double value in p_Values)
                m_Writer.Write(value);
        }

        public void Write(string[] p_Values)
        {
            if (p_Values == null)
                throw new ArgumentNullException(nameof(p_Values));

            m_Writer.Write(p_Values.Length);

            foreach (string value in p_Values)
                m_Writer.Write(value);
        }

        public void Write(Path[] p_Values)
        {
            if (p_Values == null)
                throw new ArgumentNullException(nameof(p_Values));

            m_Writer.Write(p_Values.Length);

            foreach (Path value in p_Values)
                m_Writer.Write(value.String());
        }

        public void Write<T>(T p_Value) where T : struct
        {
            Type type = typeof(T);

            if (type.IsEnum)
            {
                WriteEnum(p_Value, type);
                return;
            }

            WriteStruct(p_Value);
        }

        private void WriteEnum<T>(T p_Value, Type p_Type)
        {
            Type underlying = Enum.GetUnderlyingType(p_Type);

            if (underlying == typeof(byte))
                Write(Convert.ToByte(p_Value));
            else if (underlying == typeof(sbyte))
                Write(Convert.ToSByte(p_Value));
            else if (underlying == typeof(short))
                Write(Convert.ToInt16(p_Value));
            else if (underlying == typeof(ushort))
                Write(Convert.ToUInt16(p_Value));
            else if (underlying == typeof(int))
                Write(Convert.ToInt32(p_Value));
            else if (underlying == typeof(uint))
                Write(Convert.ToUInt32(p_Value));
            else if (underlying == typeof(long))
                Write(Convert.ToInt64(p_Value));
            else if (underlying == typeof(ulong))
                Write(Convert.ToUInt64(p_Value));
            else
                throw new NotSupportedException("Unsupported enum underlying type.");
        }

        private void WriteStruct<T>(T p_Value) where T : struct
        {
            Type type = typeof(T);

            if (type.IsAutoLayout)
            {
                throw new NotSupportedException("Struct must use sequential or explicit layout.");
            }

            int size   = Marshal.SizeOf(type);
            IntPtr ptr = Marshal.AllocHGlobal(size);

            try
            {
                Marshal.StructureToPtr(p_Value, ptr, false);

                byte[] bytes = new byte[size];
                Marshal.Copy(ptr, bytes, 0, size);

                m_Writer.Write(bytes);
            }
            finally
            {
                Marshal.FreeHGlobal(ptr);
            }
        }

        public void Clear()
        {
            EnsureNotDisposed();

            m_Writer.Flush();
            m_Stream.SetLength(0);
            m_Stream.Position = 0;
        }

        public bool Save(Path p_Path)
        {
            if (p_Path.Empty())
                return false;

            EnsureNotDisposed();
            m_Writer.Flush();

            return FileSystem.WriteFileBinary(p_Path, m_Stream.ToArray());
        }

        private void EnsureNotDisposed()
        {
            if (m_Disposed)
                throw new ObjectDisposedException(nameof(BinaryFileWriter));
        }

        public void Dispose()
        {
            if (m_Disposed)
                return;

            m_Disposed = true;
            m_Writer.Dispose();
            m_Stream.Dispose();
        }
    }

    public sealed class BinaryFileReader : IDisposable
    {
        private readonly MemoryStream m_Stream;
        private readonly System.IO.BinaryReader m_Reader;
        private bool m_Disposed;

        public long Position
        {
            get { return m_Stream.Position; }
        }

        public long Length
        {
            get { return m_Stream.Length; }
        }

        public long Remaining
        {
            get { return m_Stream.Length - m_Stream.Position; }
        }

        public BinaryFileReader(byte[] p_Data)
        {
            if (p_Data == null)
                throw new ArgumentNullException(nameof(p_Data));

            m_Stream = new MemoryStream(p_Data, false);
            m_Reader = new System.IO.BinaryReader(m_Stream);
        }

        public static BinaryFileReader FromFile(Path p_Path)
        {
            if (p_Path.Empty() || !p_Path.Exists())
                throw new FileNotFoundException("File not found: " + p_Path);

            byte[] data = FileSystem.ReadFileBinary(p_Path);

            if (data == null)
                throw new IOException("Unable to read binary file: " + p_Path);

            return new BinaryFileReader(data);
        }

        public byte ReadByte() { return m_Reader.ReadByte(); }
        public sbyte ReadSByte() { return m_Reader.ReadSByte(); }
        public short ReadInt16() { return m_Reader.ReadInt16(); }
        public ushort ReadUInt16() { return m_Reader.ReadUInt16(); }
        public int ReadInt32() { return m_Reader.ReadInt32(); }
        public uint ReadUInt32() { return m_Reader.ReadUInt32(); }
        public long ReadInt64() { return m_Reader.ReadInt64(); }
        public ulong ReadUInt64() { return m_Reader.ReadUInt64(); }
        public float ReadSingle() { return m_Reader.ReadSingle(); }
        public double ReadDouble() { return m_Reader.ReadDouble(); }
        public bool ReadBoolean() { return m_Reader.ReadBoolean(); }
        public char ReadChar() { return m_Reader.ReadChar(); }
        public string ReadString() { return m_Reader.ReadString(); }
        public Path ReadPath() { return new Path(m_Reader.ReadString()); }
        public decimal ReadDecimal() { return m_Reader.ReadDecimal(); }

        public DateTime ReadDateTime()
        {
            return DateTime.FromBinary(m_Reader.ReadInt64());
        }

        public Guid ReadGuid()
        {
            return new Guid(ReadRawBytes(16));
        }

        // Reads an array whose length was written before its contents.
        public byte[] ReadBytes()
        {
            int count = ReadCount();
            return ReadRawBytes(count);
        }

        // Reads exactly p_Count bytes without consuming a length prefix.
        public byte[] ReadRawBytes(int p_Count)
        {
            EnsureNotDisposed();

            if (p_Count < 0 || p_Count > Remaining)
            {
                throw new EndOfStreamException("Requested byte count exceeds remaining data.");
            }

            byte[] data = m_Reader.ReadBytes(p_Count);

            if (data.Length != p_Count)
                throw new EndOfStreamException();

            return data;
        }

        public int[] ReadInt32Array()
        {
            int count = ReadCount();

            if (count > Remaining / 4)
                throw new EndOfStreamException();

            int[] values = new int[count];

            for (int i = 0; i < count; i++)
                values[i] = ReadInt32();

            return values;
        }

        public float[] ReadSingleArray()
        {
            int count = ReadCount();

            if (count > Remaining / 4)
                throw new EndOfStreamException();

            float[] values = new float[count];

            for (int i = 0; i < count; i++)
                values[i] = ReadSingle();

            return values;
        }

        public double[] ReadDoubleArray()
        {
            int count = ReadCount();

            if (count > Remaining / 8)
                throw new EndOfStreamException();

            double[] values = new double[count];

            for (int i = 0; i < count; i++)
                values[i] = ReadDouble();

            return values;
        }

        public string[] ReadStringArray()
        {
            int count = ReadCount();

            if (count > Remaining)
                throw new EndOfStreamException();

            string[] values = new string[count];

            for (int i = 0; i < count; i++)
                values[i] = ReadString();

            return values;
        }

        public Path[] ReadPathArray()
        {
            int count = ReadCount();

            if (count > Remaining)
                throw new EndOfStreamException();

            Path[] values = new Path[count];

            for (int i = 0; i < count; i++)
                values[i] = ReadPath();

            return values;
        }

        public T Read<T>() where T : struct
        {
            Type type = typeof(T);

            if (type.IsEnum)
            {
                Type underlying = Enum.GetUnderlyingType(type);
                object value;

                if (underlying == typeof(byte))
                    value = ReadByte();
                else if (underlying == typeof(sbyte))
                    value = ReadSByte();
                else if (underlying == typeof(short))
                    value = ReadInt16();
                else if (underlying == typeof(ushort))
                    value = ReadUInt16();
                else if (underlying == typeof(int))
                    value = ReadInt32();
                else if (underlying == typeof(uint))
                    value = ReadUInt32();
                else if (underlying == typeof(long))
                    value = ReadInt64();
                else if (underlying == typeof(ulong))
                    value = ReadUInt64();
                else
                    throw new NotSupportedException("Unsupported enum underlying type.");

                return (T)Enum.ToObject(type, value);
            }

            return ReadStruct<T>();
        }

        private T ReadStruct<T>() where T : struct
        {
            Type type = typeof(T);

            if (type.IsAutoLayout)
            {
                throw new NotSupportedException("Struct must use sequential or explicit layout.");
            }

            int size     = Marshal.SizeOf(type);
            byte[] bytes = ReadRawBytes(size);
            IntPtr ptr   = Marshal.AllocHGlobal(size);

            try
            {
                Marshal.Copy(bytes, 0, ptr, size);

                return (T)Marshal.PtrToStructure(ptr, type);
            }
            finally
            {
                Marshal.FreeHGlobal(ptr);
            }
        }

        private int ReadCount()
        {
            int count = ReadInt32();

            if (count < 0)
            {
                throw new ArgumentOutOfRangeException("Negative collection length.");
            }

            return count;
        }

        public void Seek(long p_Position)
        {
            EnsureNotDisposed();

            if (p_Position < 0 || p_Position > m_Stream.Length)
                throw new ArgumentOutOfRangeException(nameof(p_Position));

            m_Stream.Position = p_Position;
        }

        public void Reset()
        {
            Seek(0);
        }

        private void EnsureNotDisposed()
        {
            if (m_Disposed)
                throw new ObjectDisposedException(nameof(BinaryFileReader));
        }

        public void Dispose()
        {
            if (m_Disposed)
                return;

            m_Disposed = true;
            m_Reader.Dispose();
            m_Stream.Dispose();
        }
    }

} // namespace KTN.IO
