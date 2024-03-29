#pragma once

#include "doom_map.h"
#include "raylib.h"

namespace DoomRender
{
    void DrawMapSectorPolygons(const WADFile::LevelMap& map, size_t selectedSector);
    void DrawMapSegs(const WADFile::LevelMap& map, size_t selectedSector, size_t selectedSubSector);

    void DrawMap3d(const WADFile::LevelMap& map);

    Texture2D GetTexture(const std::string& name, const WADFile& wad);
}