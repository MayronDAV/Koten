using KTN;
using KTN.IO;
using System;
using System.Collections.Generic;
using System.Numerics;
using TW.GEN;


namespace TW
{
    public class Gen2 : ScriptBehavior
    {
        public Asset m_Door;
        public Asset m_Ground;
        public Asset m_Wall;
        public Asset m_Player;

        void InstantiateMap(List<MapRect> p_Rectangles)
        {
            var rects = new Dictionary<char, List<MapRect>>();
            foreach (MapRect rect in p_Rectangles)
            {
                if (!rects.ContainsKey(rect.Type))
                    rects[rect.Type] = new List<MapRect>();

                rects[rect.Type].Add(rect);
            }

            foreach (List<MapRect> rectList in rects.Values)
            {
                if (rectList[0].Type != 'D')
                    continue;

                foreach (MapRect rect in rectList)
                {
                    Instantiate(m_Door, rect.Position, rect.Scale, Vector3.Zero);
                }
            }

            foreach (List<MapRect> rectList in rects.Values)
            {
                if (rectList[0].Type != '#')
                    continue;

                foreach (MapRect rect in rectList)
                {
                    Instantiate(m_Wall, rect.Position, rect.Scale, Vector3.Zero);
                }
            }

            foreach (List<MapRect> rectList in rects.Values)
            {
                if (rectList[0].Type != '.')
                    continue;

                foreach (MapRect rect in rectList)
                {
                    Instantiate(m_Ground, rect.Position, rect.Scale, Vector3.Zero);
                }
            }
        }

        void OnCreate()
        {
            Path iniPath    = new Path("Assets/Maps/Ini.txt");
            Path mapPath    = new Path("Assets/Maps/Map.txt");
            Path mapBinPath = new Path("Assets/Maps/Map.bin");

            if (!iniPath.Exists())
            {
                FileEmitter emitter = new FileEmitter();
                emitter.BeginBlock("Settings");
                {
                    emitter.Write("ReadFile", "bin");
                }
                emitter.EndBlock();
                bool saved = emitter.Save(iniPath);
                if (!saved)
                {
                    Console.WriteLine("Failed to save map gen ini!");
                }
            }

            if (!mapPath.Exists())
            {
                FileEmitter emitter = new FileEmitter();
                emitter.BeginBlock("MapSettings");
                {
                    emitter.Write("Map", new string[]
                    {
                        "#####D#####",
                        "#.........#",
                        "#.........#",
                        "#.........#",
                        "#..#......#",
                        "#..#......#",
                        "#..######.#",
                        "#.........#",
                        "#.........#",
                        "#.........#",
                        "###########"
                    });
                    emitter.Write("Width", "11");
                    emitter.Write("Height", "11");
                    emitter.Write("CellSize", "1");
                }
                emitter.EndBlock();
                bool saved = emitter.Save(mapPath);
                if (!saved)
                {
                    Console.WriteLine("Failed to save map!");
                }
            }

            if (!mapBinPath.Exists())
            {
                using (var writer = new BinaryFileWriter())
                {
                    writer.Write(1);  // Version
                    writer.Write(11); // Width
                    writer.Write(11); // Height
                    writer.Write(1);  // CellSize
                    writer.Write(new string[]
                    {
                        "#####D#####",
                        "#.........#",
                        "#.........#",
                        "#.....##..#",
                        "#..#......#",
                        "#..#......#",
                        "#..#####..#",
                        "#..#......#",
                        "#.........#",
                        "#.........#",
                        "###########"
                    });

                    bool saved = writer.Save(mapBinPath);
                    if (!saved)
                    {
                        Console.WriteLine("Failed to save map bin!");
                    }
                }

            }

            if (m_Door == null   || !m_Door.IsValid()   || 
                m_Ground == null || !m_Ground.IsValid() || 
                m_Wall == null   || !m_Wall.IsValid()   || 
                m_Player == null || !m_Player.IsValid())
            {
                return;
            }

            Instantiate(m_Player, new Vector3(5.0f, -8.0f, 0.0f), Vector3.One, Vector3.Zero);

            if (!iniPath.Exists())
            {
                Console.WriteLine("Ini file not found!");
                return;
            }

            FileNode rootIni  = FileReader.ReadFile(iniPath);
            FileNode settings = rootIni.GetNode("Settings");
            if (settings == null)
            {
                Console.WriteLine("Failed to read map gen ini!");
                return;
            }
            string readFile   = settings.GetString("ReadFile");

            if (readFile.Equals("txt", StringComparison.OrdinalIgnoreCase))
            {
                if (!mapPath.Exists())
                {
                    Console.WriteLine("Map file not found!");
                    return;
                }

                FileNode root        = FileReader.ReadFile(mapPath);
                FileNode mapSettings = root.GetNode("MapSettings"); 
                if (mapSettings == null)
                {
                    Console.WriteLine("Failed to read map file!");
                    return;
                }
                string[] map         = mapSettings.GetStringArray("Map");
                string widthStr      = mapSettings.GetString("Width");
                string heightStr     = mapSettings.GetString("Height");
                string cellSizeStr   = mapSettings.GetString("CellSize");

                if (int.TryParse(widthStr, out int width) && int.TryParse(heightStr, out int height))
                {
                    int size = 1;
                    if (int.TryParse(cellSizeStr, out int cellSize))
                        size = cellSize;

                    List<MapRect> rectangles = MapGenerator.Generate(map, width, size);
                    InstantiateMap(rectangles);
                }
                else
                {
                    Console.WriteLine($"Failed to parse Width or Height! the string is W: \"{widthStr}\" and H: \"{heightStr}\"");
                }  
            }
            else if (readFile.Equals("bin", StringComparison.OrdinalIgnoreCase))
            {
                if (!mapBinPath.Exists())
                {
                    Console.WriteLine("Map bin file not found!");
                    return;
                }

                using (var reader = BinaryFileReader.FromFile(mapBinPath))
                {
                    int version  = reader.ReadInt32();
                    if (version != 1)
                    {
                        Console.WriteLine($"Unsupported map version: {version}");
                        return;
                    }

                    int width    = reader.ReadInt32();
                    int height   = reader.ReadInt32();
                    int cellSize = reader.ReadInt32();
                    string[] map = reader.ReadStringArray();

                    List<MapRect> rectangles = MapGenerator.Generate(map, width, cellSize);
                    InstantiateMap(rectangles);
                }
            }
        }

        void OnUpdate()
        {
        }
    }

} // namespace TW