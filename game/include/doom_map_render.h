#pragma once

#include "doom_map.h"

namespace DoomRender
{
    void DrawMapLines(const DoomMap& map);
    void DrawMapSectorPolygons(const DoomMap& map);
}