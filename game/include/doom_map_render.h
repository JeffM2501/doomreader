#pragma once

#include "doom_map.h"

namespace DoomRender
{
    void DrawMapLines(const WADFile::LevelMap& map);
    void DrawMapSectorPolygons(const WADFile::LevelMap& map, size_t selectedSector);
}