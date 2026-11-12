using KTN;
using System;
using System.Collections.Generic;
using System.Numerics;
using TW.GEN;


namespace TW
{
    public class Gen : ScriptBehavior
    {
        public Asset m_Door;
        public Asset m_Ground;
        public Asset m_Wall;
        public Asset m_Player;

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

            if (m_Door == null   || !m_Door.IsValid()   || 
                m_Ground == null || !m_Ground.IsValid() || 
                m_Wall == null   || !m_Wall.IsValid()   || 
                m_Player == null || !m_Player.IsValid())
            {
                return;
            }

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