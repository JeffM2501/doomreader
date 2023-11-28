#include "doom_map.h"

#include "reader.h"

bool IsMapLump(const std::string& name)
{
	if (name == WADData::THINGS)
		return true;
	if (name == WADData::LINEDEFS)
		return true;
	if (name == WADData::SIDEDEFS)
		return true;
	if (name == WADData::VERTEXES)
		return true;
	if (name == WADData::SEGS)
		return true;
	if (name == WADData::SSECTORS)
		return true;
	if (name == WADData::NODES)
		return true;
	if (name == WADData::SECTORS)
		return true;
	if (name == WADData::REJECT)
		return true;
	if (name == WADData::BLOCKMAP)
		return true;

	if (name == WADData::GL_VERT)
		return true;
	if (name == WADData::GL_SEGS)
		return true;
	if (name == WADData::GL_SSECT)
		return true;
	if (name == WADData::GL_NODES)
		return true;
	if (name == WADData::GL_PVS)
		return true;

	if (name.substr(0, 3) == "GL_")
		return true;

	return false;
}

void WADFile::Read(const char* fileName)
{
	if (BufferData)
		UnloadFileData(BufferData);

	int size = 0;
	BufferData = LoadFileData(fileName, &size);

	auto rawDirectory = WADReader::ReadDirectoryEntries(BufferData);

	Levels.clear();

	LevelMap map(*this);
	bool inMap = false;

	// parse out the maps
	for (auto& entry : rawDirectory)
	{
		bool add = true;

		if (!IsMapLump(entry.Name))
			Entries[entry.Name] = entry;

		if (entry.LumpSize == 0)
		{
			if (inMap)
			{
				if (map.Entries.size() > 0)
					Levels.push_back(map);
				map.Name.clear();
				map.Entries.clear();
			}
			map.Name = entry.Name;
			inMap = true;
		}
		else
		{
			if (inMap)
			{
				if (IsMapLump(entry.Name))
				{
					map.Entries[entry.Name] = entry;
				}
				else
				{
					if (map.Entries.size() > 0)
						Levels.push_back(map);
					map.Name.clear();
					map.Entries.clear();
					inMap = false;
				}
			}
			else
			{
				if (entry.Name == WADData::PLAYPAL)
				{
					LumpDB.LoadLumpData(entry);
					PalettesLump = LumpDB.GetLump<WADData::PlayPalLump>(WADData::PLAYPAL);
				}
			}
		}
	}
}

void WADFile::LevelMap::FindLeafs(size_t nodeId)
{
	const auto& node = Nodes->Contents[nodeId];

	if (node.RightChild & (1 << 15))
	{
		// it's a leaf

		uint16_t subsector = node.RightChild & ~(1 << 15);
		LeafNodes.insert(subsector);
	}
	else
	{
		FindLeafs(node.RightChild);
	}

	if (node.LeftChild & (1 << 15))
	{
		// it's a leaf
		uint16_t subsector = node.RightChild & ~(1 << 15);
		LeafNodes.insert(subsector);
	}
	else
	{
		FindLeafs(node.LeftChild);
	}
}

void WADFile::LevelMap::CacheFlat(const std::string& flatName)
{
	if (SourceWad.Flats.find(flatName) != SourceWad.Flats.end())
		return;

	auto entryItr = SourceWad.Entries.find(flatName);

	if (entryItr == SourceWad.Entries.end() || entryItr->second.LumpSize != 4096)
		return;

	uint8_t* data = entryItr->second.BufferData + entryItr->second.LumpOffset;

	Image flatImage = GenImageColor(64, 64, BLANK);
	for (int y = 0; y < 64; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			uint8_t index = *(data + (y * 64 + x));

			Color imageColr = SourceWad.PalettesLump->Contents[0].Entry[index];

			ImageDrawPixel(&flatImage, x, 64-y, imageColr);
		}
	}
	
	SourceWad.Flats[flatName] = flatImage;
}

void WADFile::LevelMap::Load()
{
	// we need to load this first since other parsers may read it's version number
	LumpDB.LoadLumpData(Entries[WADData::GL_VERT]);

	for (auto& [key,entity] : Entries)
		LumpDB.LoadLumpData(entity);

	Verts = LumpDB.GetLump<WADData::VertexesLump>(WADData::VERTEXES);
	Lines = LumpDB.GetLump<WADData::LineDefLump>(WADData::LINEDEFS);
	Things = LumpDB.GetLump<WADData::ThingsLump>(WADData::THINGS);
	Sectors = LumpDB.GetLump<WADData::SectorsLump>(WADData::SECTORS);
	Sides = LumpDB.GetLump<WADData::SideDefLump>(WADData::SIDEDEFS);
	Segs = LumpDB.GetLump<WADData::SegsLump>(WADData::SEGS);
	Subsectors = LumpDB.GetLump<WADData::SubSectorsLump>(WADData::SSECTORS);
	Nodes = LumpDB.GetLump<WADData::NodesLump>(WADData::NODES);

	GLVerts = LumpDB.GetLump<WADData::GLVertsLump>(WADData::GL_VERT);
	GLSegs = LumpDB.GetLump<WADData::GLSegsLump>(WADData::GL_SEGS);
	GLSubSectors = LumpDB.GetLump<WADData::GLSubSectorsLump>(WADData::GL_SSECT);

	SectorCache.resize(Sectors->Contents.size());

	for (size_t sectorIndex = 0; sectorIndex < Sectors->Contents.size(); sectorIndex++)
	{
		auto& sector = SectorCache[sectorIndex];
		sector.SectorIndex = sectorIndex;
		sector.Tint = Color{ (uint8_t)GetRandomValue(128,255), (uint8_t)GetRandomValue(128,255) , (uint8_t)GetRandomValue(128,255) , 255 };

		CacheFlat(Sectors->Contents[sectorIndex].FloorTexture);
		CacheFlat(Sectors->Contents[sectorIndex].CeilingTexture);
	}

	// cache the edges in a sector
	for (size_t lineIndex = 0; lineIndex < Lines->Contents.size(); lineIndex++)
	{
		auto& line = Lines->Contents[lineIndex];

		if (line.FrontSideDef != WADData::InvalidSideDefIndex)
		{
			auto& side = Sides->Contents[line.FrontSideDef];
			auto& sector = Sectors->Contents[side.SectorId];

			SectorInfo::Edge edge;
			edge.Line = lineIndex;
			edge.Reverse = false;

			edge.Side = line.FrontSideDef;
			edge.Destination = line.BackSideDef;

			SectorCache[side.SectorId].Edges.push_back(edge);
		}

		if (line.BackSideDef != WADData::InvalidSideDefIndex)
		{
			auto& side = Sides->Contents[line.FrontSideDef];
			auto& sector = Sectors->Contents[side.SectorId];

			SectorInfo::Edge edge;
			edge.Line = lineIndex;
			edge.Reverse = true;

			edge.Side = line.BackSideDef;
			edge.Destination = line.FrontSideDef;

			SectorCache[side.SectorId].Edges.push_back(edge);
		}
	}

	for (size_t subSectorId = 0; subSectorId < GLSubSectors->Contents.size(); subSectorId++)
	{
		auto& subsector = GLSubSectors->Contents[subSectorId];
		auto& firstSeg = GLSegs->Contents[subsector.StartSegment];
		auto& line = Lines->Contents[firstSeg.LineIndex];

		size_t side = line.FrontSideDef;
		if (firstSeg.Direction)
			side = line.BackSideDef;

		size_t sector = Sides->Contents[side].SectorId;

		SectorCache[sector].SubSectors.push_back(subSectorId);
	}

//	FindLeafs(Nodes->Contents.size()-1);
}

Vector2 WADFile::LevelMap::GetVertex(size_t index, bool isGLVert) const
{
	if (isGLVert)
		return Vector2{ GLVerts->Contents[index].x, GLVerts->Contents[index].y};

	return Vector2{ (float)Verts->Contents[index].X, float(Verts->Contents[index].Y) };
}

void WADFile::LumpDatabase::LoadLumpData(const WADData::DirectoryEntry& entry)
{
	if (Lumps.find(entry.Name) != Lumps.end())
		return;

	WADData::Lump* lump = WADData::GetLump(entry.Name);
	if (!lump)
		return;

	int version = 0;
	auto glVertsLump = Lumps.find(WADData::GL_VERT);
	if (glVertsLump != Lumps.end())
		version = ((WADData::GLVertsLump*)glVertsLump->second)->FormatVersion;

	lump->Parse(entry.BufferData, entry.LumpOffset, entry.LumpSize, version);
	Lumps.insert_or_assign(entry.Name, lump);
}
