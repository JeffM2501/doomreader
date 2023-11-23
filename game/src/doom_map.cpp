#include "doom_map.h"

#include "reader.h"

DoomMap::~DoomMap()
{
	if (BufferData)
		UnloadFileData(BufferData);

	for (const auto& [key, lump] : LumpDB)
	{
		if (lump)
			delete(lump);
	}

	LumpDB.clear();
}

void DoomMap::Read(const std::string& fileName)
{
	int size = 0;
	BufferData = LoadFileData("resources/E1M1.wad", &size);

	Entries = WADReader::ReadDirectoryEntries(BufferData);

	for (auto& ent : Entries)
		LoadLumpData(&ent);
}

void DoomMap::LoadLumpData(const WADData::DirectoryEntry* entry)
{
	if (!entry || LumpDB.find(entry->Name) != LumpDB.end())
		return;

	WADData::Lump* lump = WADData::GetLump(entry->Name);
	if (!lump)
		return;

	lump->Parse(BufferData, entry->LumpOffset, entry->LumpSize);
	LumpDB.insert_or_assign(entry->Name, lump);
}

void CacheMap(DoomMap& map)
{
	map.Verts = (WADData::VertexesLump*)map.LumpDB[WADData::VERTEXES];
	map.Lines = (WADData::LineDefLump*)map.LumpDB[WADData::LINEDEFS];
	map.Sectors = (WADData::SectorsLump*)map.LumpDB[WADData::SECTORS];
	map.Things = (WADData::ThingsLump*)map.LumpDB[WADData::THINGS];
}
