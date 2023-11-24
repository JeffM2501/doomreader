#include "doom_map_render.h"

#include "raylib.h"

namespace DoomRender
{
    void DrawMapLines(const WADFile::LevelMap& map)
    {
        if (map.LumpDB.size() == 0)
            return;

        for (const auto& line : map.Lines->Contents)
        {
            auto& sp = map.Verts->Contents[line.Start];
            auto& ep = map.Verts->Contents[line.End];

            DrawLine(sp.X, sp.Y, ep.X, ep.Y, WHITE);
        }

        for (const auto& thing : map.Things->Contents)
        {
            DrawCircle(thing.X, thing.Y, 10, YELLOW);
        }
    }

    void DrawMapSectorPolygons(const WADFile::LevelMap& map)
    {

    }
}