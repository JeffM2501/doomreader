#pragma once

#include "doom_map.h"

namespace DoomRender
{
    void DrawMapLines(const WADFile::LevelMap& map);
    void DrawMapSectorPolygons(const WADFile::LevelMap& map, size_t selectedSector);

    void DrawMapSegs(const WADFile::LevelMap& map, size_t selectedSector, size_t selectedSubSector);

    void DrawMapNodes(const WADFile::LevelMap& map, size_t selectedSector, size_t selectedSubSector);
}