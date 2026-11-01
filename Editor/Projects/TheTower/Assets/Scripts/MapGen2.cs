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
        private Asset m_Door;
        private Asset m_Ground;
        private Asset m_Wall;
        private Asset m_Player;

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
            if (!File.Exists("Maps/Ini.txt"))
            {
                FileEmitter emitter = new FileEmitter();
                emitter.BeginBlock("Settings");
                {
                    emitter.Write("ReadFile", "bin");
                }
                emitter.EndBlock();
                bool saved = emitter.Save("Maps/Ini.txt");
                if (!saved)
                {
                    Console.WriteLine("Failed to save map gen ini!");
                }
            }

            if (!File.Exists("Maps/Map.txt"))
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
                bool saved = emitter.Save("Maps/Map.txt");
                if (!saved)
                {
                    Console.WriteLine("Failed to save map!");
                }
            }

            if (!File.Exists("Maps/Map.bin"))
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

                    bool saved = writer.Save("Maps/Map.bin");
                    if (!saved)
                    {
                        Console.WriteLine("Failed to save map bin!");
                    }
                }

            }

            m_Door   = AssetManager.FindWithPath("Prefabs/Door.ktprefab");
            m_Ground = AssetManager.FindWithPath("Prefabs/Ground.ktprefab");
            m_Wall   = AssetManager.FindWithPath("Prefabs/Wall.ktprefab");
            m_Player = AssetManager.FindWithPath("Prefabs/Player.ktprefab");

            Instantiate(m_Player, new Vector3(5.0f, -8.0f, 0.0f), Vector3.One, Vector3.Zero);

            FileNode rootIni  = FileReader.ReadFile("Maps/Ini.txt");
            FileNode settings = rootIni.GetNode("Settings");
            string readFile   = settings.GetString("ReadFile");

            if (readFile.Equals("txt", StringComparison.OrdinalIgnoreCase))
            {
                FileNode root        = FileReader.ReadFile("Maps/Map.txt");

                FileNode mapSettings = root.GetNode("MapSettings"); 
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
                using (var reader = BinaryFileReader.FromFile("Maps/Map.bin"))
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