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

	return false;
}

void WADFile::Read(const char* fileName)
{
	if (BufferData)
		UnloadFileData(BufferData);

	int size = 0;
	BufferData = LoadFileData(fileName, &size);

	Entries = WADReader::ReadDirectoryEntries(BufferData);

	Levels.clear();

	LevelMap map;
	bool inMap = false;

	// parse out the maps
	for (auto& entry : Entries)
	{
		if (entry.LumpSize == 0)
		{
			if (inMap)
			{
				if (map.Entries.size() > 0)
					Levels.push_back(map);
				map.Name.clear();
				map.Entries.clear();
			}
			map.BufferData = BufferData;
			map.Name = entry.Name;
			inMap = true;
		}
		else
		{
			if (inMap)
			{
				if (IsMapLump(entry.Name))
				{
					map.Entries.push_back(entry);
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
		}
	}
}

void WADFile::LevelMap::Load()
{
	for (auto& entity : Entries)
		LoadLumpData(entity);

	Verts = GetLump<WADData::VertexesLump>(WADData::VERTEXES);
	Lines = GetLump<WADData::LineDefLump>(WADData::LINEDEFS);
	Things = GetLump<WADData::ThingsLump>(WADData::THINGS);
	Sectors = GetLump<WADData::SectorsLump>(WADData::SECTORS);
	Sides = GetLump<WADData::SideDefLump>(WADData::SIDEDEFS);

	SectorCache.resize(Sectors->Contents.size());

	for (auto& sector : SectorCache)
		sector.Tint = Color{ (uint8_t)GetRandomValue(128,255), (uint8_t)GetRandomValue(128,255) , (uint8_t)GetRandomValue(128,255) , 255 };

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


}

void WADFile::LevelMap::LoadLumpData(const WADData::DirectoryEntry& entry)
{
	if (LumpDB.find(entry.Name) != LumpDB.end())
		return;

	WADData::Lump* lump = WADData::GetLump(entry.Name);
	if (!lump)
		return;

	lump->Parse(BufferData, entry.LumpOffset, entry.LumpSize);
	LumpDB.insert_or_assign(entry.Name, lump);
}
