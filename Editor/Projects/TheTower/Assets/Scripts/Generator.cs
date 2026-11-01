using KTN;
using System.Collections.Generic;

namespace TW.GEN
{
    public struct MapRect
    {
        public char Type;

        public int MinX;
        public int MinY;
        public int MaxX;
        public int MaxY;

        public Vector3 Position;
        public Vector3 Scale;
    }

    public static class MapGenerator
    {
        public static List<MapRect> Generate(string[] p_Map, int p_Width, float p_CellSize = 1.0f)
        {
            if (p_Map == null)
                return new List<MapRect>();

            int height = p_Map.Length;
            if (p_Width <= 0 || height <= 0)
            {
                return new List<MapRect>();
            }

            bool[,] visited = new bool[height, p_Width];

            var rectangles = new List<MapRect>();

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < p_Width; x++)
                {
                    if (visited[y, x])
                        continue;

                    char type = p_Map[y][x];

                    int rectWidth = 1;
                    while (x + rectWidth < p_Width)
                    {
                        int nextX = x + rectWidth;
                        if (visited[y, nextX] || p_Map[y][nextX] != type)
                            break;

                        rectWidth++;
                    }

                    int rectHeight = 1;
                    while (y + rectHeight < height)
                    {
                        int nextY = y + rectHeight;
                        bool canExpand = true;

                        for (int dx = 0; dx < rectWidth; dx++)
                        {
                            if (visited[nextY, x + dx] || p_Map[nextY][x + dx] != type)
                            {
                                canExpand = false;
                                break;
                            }
                        }

                        if (!canExpand)
                            break;

                        rectHeight++;
                    }

                    for (int dy = 0; dy < rectHeight; dy++)
                    {
                        for (int dx = 0; dx < rectWidth; dx++)
                            visited[y + dy, x + dx] = true;
                    }

                    int minX = x;
                    int minY = y;
                    int maxX = x + rectWidth - 1;
                    int maxY = y + rectHeight - 1;

                    float centerX = (minX + maxX) * 0.5f * p_CellSize;
                    float centerY = (minY + maxY) * 0.5f * p_CellSize;

                    Vector3 scale = new Vector3(
                        rectWidth * p_CellSize,
                        rectHeight * p_CellSize,
                        1.0f
                    );

                    rectangles.Add(new MapRect
                    {
                        Type = type,

                        MinX = minX,
                        MinY = minY,
                        MaxX = maxX,
                        MaxY = maxY,

                        Position = new Vector3(
                            centerX,
                            -centerY,
                            0.0f
                        ),

                        Scale = scale
                    });
                }
            }

            return rectangles;
        }

        public static List<MapRect> Generate(string p_Map, int p_Width, int p_Height, float p_CellSize = 1.0f)
        {
            if (p_Map == null)
                return new List<MapRect>();

            if (p_Width <= 0 || p_Height <= 0 || p_Map.Length == 0 ||
                p_Map.Length != (p_Width * p_Height))
            {
                return new List<MapRect>();
            }

            var newMap = new string[p_Height];
            for (int y = 0; y < p_Height; y++)
            {
                string row = "";
                for (int x = 0; x < p_Width; x++)
                {
                    row += p_Map[y * p_Width + x];
                }

                newMap[y] = row;
            }

            return Generate(newMap, p_Width, p_CellSize);
        }
    }

} // namespace TW.GEN