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
			DrawCircleV(thing.Position, 0.25f, YELLOW);
		}
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

                const auto& sp = map.Verts->Contents[line.Start].Position;
                const auto& ep = map.Verts->Contents[line.End].Position;

				Color c = WHITE;
				if (edge.Reverse)
				{
					if (edge.Destination != WADData::InvalidSectorIndex)
						c = BLUE;
					else
						c = DARKBLUE;
				}
				else
				{
                    if (edge.Destination != WADData::InvalidSectorIndex)
                        c = GREEN;
                    else
                        c = DARKGREEN;
				}

                if (edge.Reverse)
                    DrawLineEx(sp, ep, 0.125f,  c);
                else
					DrawLineEx(ep, sp, 0.125f, c);
            }
        }
        rlDrawRenderBatchActive();
        if (selectedSector < map.SectorCache.size())
        {
            for (const auto& edge : map.SectorCache[selectedSector].Edges)
            {
				const auto& line = map.Lines->Contents[edge.Line];

				const auto& sp = map.Verts->Contents[line.Start].Position;
				const auto& ep = map.Verts->Contents[line.End].Position;

				if (edge.Reverse)
					DrawLineEx(sp, ep, 0.125f, RED);
				else
					DrawLineEx(ep, sp, 0.125f, RED);
            }

            rlDrawRenderBatchActive();
        }
        DrawThigs(map);
    }

	void DrawMapSegs(const WADFile::LevelMap& map, size_t selectedSector, size_t selectedSubSector)
	{
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

                Vector2 origin = map.GetVertex(map.GLSegs->Contents[glSubSector.StartSegment].Start, map.GLSegs->Contents[glSubSector.StartSegment].StartIsGL);

				for (size_t index = glSubSector.StartSegment+1; index < glSubSector.StartSegment + glSubSector.Count; index++)
				{
					const auto& segment = map.GLSegs->Contents[index];

                    Vector2 sp = map.GetVertex(segment.Start, segment.StartIsGL);
					Vector2 ep = map.GetVertex(segment.End, segment.EndIsGL);

                    rlTexCoord2f(origin.x/2.0f, origin.y / 2.0f);
                    rlVertex2f(origin.x, origin.y);
                    rlVertex2f(origin.x, origin.y);

					rlTexCoord2f(sp.x / 2.0f, sp.y / 2.0f);
					rlVertex2f(sp.x, sp.y);

					rlTexCoord2f(ep.x / 2.0f, ep.y / 2.0f);
					rlVertex2f(ep.x, ep.y);
				}
            }

            rlEnd();

            rlDrawRenderBatchActive();
            rlSetTexture(0);
        }

        DrawMapSectorPolygons(map, selectedSector);

		if (selectedSector < map.SectorCache.size())
		{
			const auto& sector = map.SectorCache[selectedSector];

			for (size_t i = 0; i < sector.SubSectors.size(); i++)
			{
				size_t subSectorIndex = sector.SubSectors[i];

				if (i != selectedSubSector)
					continue;

				const auto& subSector = map.GLSubSectors->Contents[subSectorIndex];

				for (size_t index = subSector.StartSegment; index < subSector.StartSegment + subSector.Count; index++)
				{
					const auto& segment = map.GLSegs->Contents[index];

					Vector2 sp = map.GetVertex(segment.Start, segment.StartIsGL);
					Vector2 ep = map.GetVertex(segment.End, segment.EndIsGL);

					DrawLineEx(sp, ep, 0.15f, PURPLE);
				}
				break;
			}
		}
	}

	void DrawMap3d(const WADFile::LevelMap& map)
	{
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

				float lightLevel = (rawSector.LightLevel / 255.0f) * 0.75f;
				rlColor4f(lightLevel, lightLevel, lightLevel, 1);

				Vector2 origin = map.GetVertex(map.GLSegs->Contents[glSubSector.StartSegment].Start, map.GLSegs->Contents[glSubSector.StartSegment].StartIsGL);

				for (size_t index = glSubSector.StartSegment + 1; index < glSubSector.StartSegment + glSubSector.Count; index++)
				{
					const auto& segment = map.GLSegs->Contents[index];

					Vector2 sp = map.GetVertex(segment.Start, segment.StartIsGL);
					Vector2 ep = map.GetVertex(segment.End, segment.EndIsGL);

					rlTexCoord2f(ep.x / 2.0f, ep.y / 2.0f);
					rlVertex3f(ep.x, ep.y, rawSector.Floor);

					rlTexCoord2f(sp.x / 2.0f, sp.y / 2.0f);
					rlVertex3f(sp.x, sp.y, rawSector.Floor);

					rlTexCoord2f(origin.x / 2.0f, origin.y / 2.0f);
					rlVertex3f(origin.x, origin.y, rawSector.Floor);
					rlVertex3f(origin.x, origin.y, rawSector.Floor);
				}
			}

			rlEnd();

			rlDrawRenderBatchActive();

			Texture2D ceiling = GetFlat(rawSector.CeilingTexture, map.SourceWad);
			rlSetTexture(ceiling.id);

			rlBegin(RL_QUADS);
			rlColor4f(1, 1, 1, 1);
			rlNormal3f(0, 0, 1);

			for (size_t subsectorIndex : sector.SubSectors)
			{
				const auto& glSubSector = map.GLSubSectors->Contents[subsectorIndex];

				float lightLevel = rawSector.LightLevel / 255.0f;
				rlColor4f(lightLevel, lightLevel, lightLevel, 1);

				Vector2 origin = map.GetVertex(map.GLSegs->Contents[glSubSector.StartSegment].Start, map.GLSegs->Contents[glSubSector.StartSegment].StartIsGL);

				for (size_t index = glSubSector.StartSegment + 1; index < glSubSector.StartSegment + glSubSector.Count; index++)
				{
					const auto& segment = map.GLSegs->Contents[index];

					Vector2 sp = map.GetVertex(segment.Start, segment.StartIsGL);
					Vector2 ep = map.GetVertex(segment.End, segment.EndIsGL);

					rlTexCoord2f(origin.x / 2.0f, origin.y / 2.0f);
					rlVertex3f(origin.x, origin.y, rawSector.Ceiling);
					rlVertex3f(origin.x, origin.y, rawSector.Ceiling);

					rlTexCoord2f(sp.x / 2.0f, sp.y / 2.0f);
					rlVertex3f(sp.x, sp.y, rawSector.Ceiling);

					rlTexCoord2f(ep.x / 2.0f, ep.y / 2.0f);
					rlVertex3f(ep.x, ep.y, rawSector.Ceiling);
				}
			}

			rlEnd();

			rlDrawRenderBatchActive();
			rlSetTexture(0);
		}

		for (const auto& thing : map.Things->Contents)
		{
			float floor = 0;
			if (thing.SectorId != size_t(-1))
				floor = map.Sectors->Contents[thing.SectorId].Floor;
			DrawSphere(Vector3{ thing.Position.x, thing.Position.y, floor + 0.5f }, 0.125f, ColorAlpha(YELLOW, 0.25f));
		}

		rlSetTexture(0);
		rlBegin(RL_QUADS);
		
		for (const auto& sector : map.SectorCache)
		{
			for (const auto& edge : sector.Edges)
			{
				const auto& line = map.Lines->Contents[edge.Line];

				auto sp = map.Verts->Contents[line.Start].Position;
				auto ep = map.Verts->Contents[line.End].Position;

				if (false && edge.Reverse)
				{
					auto t = ep;
					ep = sp;
					sp = ep;
				}

				if (edge.Destination != WADData::InvalidSideDefIndex)
				{
				}
				else // it's a full wall
				{
					float floor = map.Sectors->Contents[sector.SectorIndex].Floor;
					float ceiling = map.Sectors->Contents[sector.SectorIndex].Ceiling;

					uint8_t alpha = 1;
					if (edge.Destination != WADData::InvalidSideDefIndex)
						alpha = 64;

					rlColor4ub(sector.Tint.r, sector.Tint.g, sector.Tint.b, alpha);
					rlVertex3f(sp.x, sp.y, floor);
					rlVertex3f(ep.x, ep.y, floor);
					rlVertex3f(ep.x, ep.y, ceiling);
					rlVertex3f(sp.x, sp.y, ceiling);
				}
	
			}
		}
		rlEnd();
	}

}