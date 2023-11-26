#include "doom_map_render.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

namespace DoomRender
{
    std::unordered_map<std::string, Texture2D> FlatCache;

    Texture2D GetFlat(const std::string& name, const WADFile& wad)
    {
        auto itr = FlatCache.find(name);
        if (itr != FlatCache.end())
            return itr->second;

        auto imageItr = wad.Flats.find(name);
        if (imageItr == wad.Flats.end())
            return Texture2D{ 0 };

        Texture2D texture = LoadTextureFromImage(imageItr->second);
        FlatCache[name] = texture;
        return texture;
    }

    void DrawThigs(const WADFile::LevelMap& map)
    {
		for (const auto& thing : map.Things->Contents)
		{
			DrawCircle(thing.X, thing.Y, 10, YELLOW);
		}
    }

    void DrawMapLines(const WADFile::LevelMap& map)
    {
        if (map.Verts == nullptr)
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
        if (map.Verts == nullptr)
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

        for (const auto& sector : map.SectorCache)
        {
            auto& rawSector = map.Sectors->Contents[sector.SectorIndex];
            Texture2D floor = GetFlat(rawSector.FloorTexture, map.SourceWad);
            rlSetTexture(floor.id);

            rlBegin(RL_QUADS);
            rlColor4f(1, 1, 1, 1);
            rlNormal3f(0, 0, 1);

            for (size_t subsectorIndex : sector.SubSectors)
            {
                const auto& glSubSector = map.GLSubSectors->Contents[subsectorIndex];

                float lightLevel = rawSector.LightLevel / 255.0f;
                rlColor4f(lightLevel, lightLevel, lightLevel, 1);

                Vector2 origin = map.GetVertex(map.GLSegs->Contents[glSubSector.StartSegment].Start, map.GLSegs->Contents[glSubSector.StartSegment].SartIsGL);

				for (size_t index = glSubSector.StartSegment+1; index < glSubSector.StartSegment + glSubSector.Count; index++)
				{
					const auto& segment = map.GLSegs->Contents[index];

                    Vector2 sp = map.GetVertex(segment.Start, segment.SartIsGL);
					Vector2 ep = map.GetVertex(segment.End, segment.EndIsGL);

                    rlTexCoord2f(origin.x/64, origin.y / 64);
                    rlVertex2f(origin.x, origin.y);
                    rlVertex2f(origin.x, origin.y);

					rlTexCoord2f(sp.x / 64, sp.y / 64);
					rlVertex2f(sp.x, sp.y);

					rlTexCoord2f(ep.x / 64, ep.y / 64);
					rlVertex2f(ep.x, ep.y);
				}
            }

            rlEnd();

            rlDrawRenderBatchActive();
            rlSetTexture(0);
        }

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

					DrawLineEx(sp, ep, 2, ColorAlpha(YELLOW, 0.5f));
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