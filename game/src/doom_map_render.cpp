#include "doom_map_render.h"

#include "raylib.h"
#include "rlgl.h"

namespace DoomRender
{
    void DrawThigs(const WADFile::LevelMap& map)
    {
		for (const auto& thing : map.Things->Contents)
		{
			DrawCircle(thing.X, thing.Y, 10, YELLOW);
		}
    }

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

        DrawThigs(map);
    }

    void DrawMapSectorPolygons(const WADFile::LevelMap& map, size_t selectedSector)
    {
		if (map.LumpDB.size() == 0)
			return;

        for (const auto& sector : map.SectorCache)
        {
            for (const auto& edge : sector.Edges)
            {
                const auto& line = map.Lines->Contents[edge.Line];

                const auto& sp = map.Verts->Contents[line.Start];
                const auto& ep = map.Verts->Contents[line.End];

                if (edge.Reverse)
                    DrawLine(sp.X, sp.Y, ep.X, ep.Y, sector.Tint);
                else
                    DrawLine(ep.X, ep.Y, sp.X, sp.Y, sector.Tint);
            }
        }
        rlDrawRenderBatchActive();
        rlSetLineWidth(3);
        if (selectedSector < map.SectorCache.size())
        {
            for (const auto& edge : map.SectorCache[selectedSector].Edges)
            {
				const auto& line = map.Lines->Contents[edge.Line];

				const auto& sp = map.Verts->Contents[line.Start];
				const auto& ep = map.Verts->Contents[line.End];

				if (edge.Reverse)
					DrawLine(sp.X, sp.Y, ep.X, ep.Y, RED);
				else
					DrawLine(ep.X, ep.Y, sp.X, sp.Y, RED);
            }

            rlDrawRenderBatchActive();
        }
       
        rlSetLineWidth(1);

        DrawThigs(map);
    }
}