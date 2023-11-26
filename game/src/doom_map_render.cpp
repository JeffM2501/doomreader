#include "doom_map_render.h"

#include "raylib.h"
#include "raymath.h"
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
        if (map.SourceWad.LumpDB.size() == 0)
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
		if (map.SourceWad.LumpDB.size() == 0)
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

        for (const auto& glVert : map.GLVerts->Contents)
        {
            DrawCircleV(Vector2{ glVert.X, glVert.Y }, 5, PINK);
        }

        DrawThigs(map);
    }

	void DrawMapSegs(const WADFile::LevelMap& map, size_t selectedSector, size_t selectedSubSector)
	{
        DrawMapSectorPolygons(map, selectedSector);

         if (selectedSector < map.SectorCache.size())
         {
             const auto& sector = map.SectorCache[selectedSector];

            for (size_t i = 0; i < sector.SubSectors.size(); i++)
            {
                size_t subSectorIndex = sector.SubSectors[i];

                if (i == selectedSubSector)
                    continue;

                const auto& subSector = map.GLSubSectors->Contents[subSectorIndex];

                for (size_t index = subSector.StartSegment; index < subSector.StartSegment + subSector.Count; index++)
                {
                    const auto& segment = map.GLSegs->Contents[index];

                    Vector2 sp = map.GetVertex(segment.Start, segment.SartIsGL);
				    Vector2 ep = map.GetVertex(segment.End, segment.EndIsGL);

                    DrawLineEx(sp, ep,2, ColorAlpha(YELLOW ,0.5f));
                }
            }

            for (size_t i = 0; i < sector.SubSectors.size(); i++)
            {
                size_t subSectorIndex = sector.SubSectors[i];

                if (i != selectedSubSector)
                    continue;

                const auto& subSector = map.GLSubSectors->Contents[subSectorIndex];

                for (size_t index = subSector.StartSegment; index < subSector.StartSegment + subSector.Count; index++)
                {
                    const auto& segment = map.GLSegs->Contents[index];

                    Vector2 sp = map.GetVertex(segment.Start, segment.SartIsGL);
                    Vector2 ep = map.GetVertex(segment.End, segment.EndIsGL);

                    DrawLineEx(sp, ep, 5, PURPLE);
                }
                break;
            }
		}
	}

    bool IsLeaf(uint16_t childId)
    {
        return childId & (1 << 15);
    }

    size_t GetSectorId(uint16_t childId)
    {
        return childId & ~(1 << 15);;
    }

    void DrawBBOX(int16_t bbox[4], Color color)
    {
        Rectangle rect = { float(bbox[2]), float(bbox[1]), float(bbox[3] - bbox[2]), float(bbox[0] - bbox[1]) };
      //  DrawRectangleLinesEx(rect, 2, color);

        DrawCircle(bbox[2], bbox[0], 10, PURPLE);
        DrawCircle(bbox[3], bbox[1], 10, DARKPURPLE);
    }

    void DrawNode(const WADFile::LevelMap& map, size_t nodeID)
    {
        auto& node = map.Nodes->Contents[nodeID];

        DrawBBOX(node.RightBBox, MAROON);
        DrawBBOX(node.LeftBBox, SKYBLUE);

        DrawLine(node.PartitionStartX, node.PartitionStartY, node.PartitionStartX + node.PartitionSlopeX, node.PartitionStartY + node.PartitionSlopeY, YELLOW);

        if (!IsLeaf(node.RightChild))
        {
            DrawNode(map, node.RightChild);
        }

		if (!IsLeaf(node.LeftChild))
		{
			DrawNode(map, node.LeftChild);
		}
    }

	void DrawMapNodes(const WADFile::LevelMap& map, size_t selectedSector, size_t selectedSubSector)
	{
        DrawMapSectorPolygons(map, selectedSector);

      //  DrawNode(map, map.Nodes->Contents.size() - 1);
	}

}