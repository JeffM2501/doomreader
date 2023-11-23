#pragma once

#include "lump_types.h"
#include "raylib.h"

#include <string>
#include <vector>
#include <unordered_map>


class DoomMap 
{
public:
	~DoomMap();

	void Read(const std::string& fileName);

	std::vector<WADData::DirectoryEntry> Entries;
	std::unordered_map<std::string, WADData::Lump*> LumpDB;

	WADData::VertexesLump* Verts = nullptr;
	WADData::LineDefLump* Lines = nullptr;
	WADData::ThingsLump* Things = nullptr;
	WADData::SectorsLump* Sectors = nullptr;

	struct SectorInfo 
	{
		struct Edge
		{
			// The line for this edge
			uint16_t Line = 0;

			// The side that we would draw if we are inside this sector
			uint16_t Side = 0;

			// The sector on the other side of this line
			uint16_t Destination = WADData::InvalidSideDefIndex;
		};

		// The sorted list of edges for this sector (forms a loop)
		std::vector<Edge> Edges;

		Color Tint = WHITE;
	};

	std::vector<SectorInfo> SectorCache;

protected:
	uint8_t* BufferData = nullptr;
	size_t BufferSize = 0;


	void LoadLumpData(const WADData::DirectoryEntry* entry);
};

void CacheMap(DoomMap& map);