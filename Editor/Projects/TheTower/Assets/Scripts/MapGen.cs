using KTN;
using System;
using System.Collections.Generic;
using System.Numerics;
using TW.GEN;


namespace TW
{
    public class Gen : ScriptBehavior
    {
        public Asset Door;
        public Asset Ground;
        public Asset Wall;
        public Asset Player;
        public bool UseVersion2 = false;
        public bool Draw100 = true;

        private string m_Map;
        private string[] m_Map2;

        void OnCreate()
        {
            m_Map    = "#####D######.........##.........##.........##..#......##..#......##..######.##.........##.........##.........############";
            m_Map2   = new string[]{
                "#####D################################",
                "#..........#.........................#",
                "#..................#........#........#",
                "#..........##........................#",
                "#...#......#........#................#",
                "#........................#........#..#",
                "#...............#........##..........#",
                "#..........#..............#..........#",
                "#.................#..................#",
                "#...#...........................#....#",
                "#.............#........#.............#",
                "#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.##",
                "##.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
                "#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.##",
                "##.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
                "#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.##",
                "##.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
                "#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.##",
                "##.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
                "#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.##",
                "##.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
                "#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.##",
                "##.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.#",
                "######################################"
            };

            if (Door == null   || !Door.IsValid()   || 
                Ground == null || !Ground.IsValid() || 
                Wall == null   || !Wall.IsValid()   || 
                Player == null || !Player.IsValid())
            {
                return;
            }

            Instantiate(Player, new Vector3(5.0f, -8.0f, 0.0f), Vector3.One, Vector3.Zero);

            if (Draw100)
            {
                for (int x = 0; x < 11; x++)
                {
                    for (int y = 0; y < 11; y++)
                    {
                        Instantiate(Ground, new Vector3(x, y, 0.0f), Vector3.One, Vector3.Zero);
                    }
                }
                return;
            }

            //List<MapRect> rectangles = MapGenerator.Generate(m_Map, 11, 11, 1.0f);
            List<MapRect> rectangles = UseVersion2 ? MapGenerator.Generate(m_Map2, 38, 1.0f) : MapGenerator.Generate(m_Map, 11, 11, 1.0f);
            var rects = new Dictionary<char, List<MapRect>>();
            foreach (MapRect rect in rectangles)
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
                    Instantiate(Door, rect.Position, rect.Scale, Vector3.Zero);
                }
            }

            foreach (List<MapRect> rectList in rects.Values)
            {
                if (rectList[0].Type != '#')
                    continue;

                foreach (MapRect rect in rectList)
                {
                    Instantiate(Wall, rect.Position, rect.Scale, Vector3.Zero);
                }
            }

            foreach (List<MapRect> rectList in rects.Values)
            {
                if (rectList[0].Type != '.')
                    continue;

                foreach (MapRect rect in rectList)
                {
                    Instantiate(Ground, rect.Position, rect.Scale, Vector3.Zero);
                }
            }
        }

        void OnUpdate()
        {
        }
    }

} // namespace TW