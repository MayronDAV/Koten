using KTN;
using System;
using System.Collections.Generic;
using System.Numerics;
using TW.GEN;


namespace TW
{
    public class Gen : ScriptBehavior
    {
        private Asset m_Door;
        private Asset m_Ground;
        private Asset m_Wall;
        private Asset m_Player;

        private string m_Map;
        private string[] m_Map2;

        void OnCreate()
        {
            m_Map    = "#####D######.........##.........##.........##..#......##..#......##..######.##.........##.........##.........############";
            m_Map2   = new string[]{
                "#####D#####",
                "#.........#",
                "#.........#",
                "#.........#",
                "#.........#",
                "#.........#",
                "#.........#",
                "#.........#",
                "#.........#",
                "#.........#",
                "#.........#",
                "###########"
            };


            m_Door   = AssetManager.FindWithPath("Prefabs/Door.ktprefab");
            m_Ground = AssetManager.FindWithPath("Prefabs/Ground.ktprefab");
            m_Wall   = AssetManager.FindWithPath("Prefabs/Wall.ktprefab");
            m_Player = AssetManager.FindWithPath("Prefabs/Player.ktprefab");

            Instantiate(m_Player, new Vector3(5.0f, -8.0f, 0.0f), new Vector3(1.0f, 1.0f, 1.0f), Vector3.Zero);

            List<MapRect> rectangles = MapGenerator.Generate(m_Map, 11, 11, 1.0f);
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

        void OnUpdate()
        {
        }
    }

} // namespace TW