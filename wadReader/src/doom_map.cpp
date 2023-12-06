#include "doom_map.h"

#include "reader.h"
#include "raymath.h"

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
			bool skip = false;
			if (inMap)
			{
				if (IsMapLump(entry.Name))
				{
					map.Entries[entry.Name] = entry;
					skip = true;
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

			if (!skip)
			{
				LumpDB.LoadLumpData(entry);

				if (entry.Name == WADData::PLAYPAL)
					PalettesLump = LumpDB.GetLump<WADData::PlayPalLump>(WADData::PLAYPAL);
				if (entry.Name == WADData::PNAMES)
					PatchNames = LumpDB.GetLump<WADData::PatchNamesLump>(WADData::PNAMES);
				if (entry.Name == WADData::TEXTURE)
					TextureLumps.push_back(LumpDB.GetLump<WADData::TexturesLump>(WADData::TEXTURE));
				if (entry.Name == WADData::TEXTURE1)
					TextureLumps.push_back(LumpDB.GetLump<WADData::TexturesLump>(WADData::TEXTURE1));
				if (entry.Name == WADData::TEXTURE2)
					TextureLumps.push_back(LumpDB.GetLump<WADData::TexturesLump>(WADData::TEXTURE2));
			}
		}
	}
}

size_t WADFile::LevelMap::GetSectorFromPoint(float x, float y, size_t* outSubSector) const
{
	Vector2 point = { x,y };

	for (const auto& sector : SectorCache)
	{
		size_t subSectorIndex = 0;

		for (size_t subSectorID : sector.SubSectors)
		{
			const auto& subSector = GLSubSectors->Contents[subSectorID];

			Vector2 origin = GetVertex(GLSegs->Contents[subSector.StartSegment].End, GLSegs->Contents[subSector.StartSegment].EndIsGL);

			for (size_t i = 1; i < subSector.Count; i++)
			{
				Vector2 sp = GetVertex(GLSegs->Contents[i + subSector.StartSegment].Start, GLSegs->Contents[i + subSector.StartSegment].StartIsGL);
				Vector2 ep = GetVertex(GLSegs->Contents[i + subSector.StartSegment].End, GLSegs->Contents[i + subSector.StartSegment].EndIsGL);

				if (CheckCollisionPointTriangle(point, origin, sp, ep))
				{
                    if (outSubSector)
                        *outSubSector = subSectorIndex;
                    return sector.SectorIndex;
				}
			}
		
			subSectorIndex++;
		}
	}

	return size_t(-1);
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

	auto& palette = SourceWad.PalettesLump->Contents[0];

	Image flatImage = GenImageColor(64, 64, BLANK);
	for (int y = 0; y < 64; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			uint8_t index = *(data + (y * 64 + x));

			Color imageColr = palette.Entry[index];

			ImageDrawPixel(&flatImage, x, 63-y, imageColr);
		}
	}
	
	SourceWad.Flats[flatName] = flatImage;
}

void WADFile::LevelMap::CachePatch(const std::string& patchName)
{
	if (SourceWad.Patches.find(patchName) != SourceWad.Patches.end())
		return;

	auto entryItr = SourceWad.Entries.find(patchName);

	if (entryItr == SourceWad.Entries.end())
		return;

	uint8_t* data = entryItr->second.BufferData + entryItr->second.LumpOffset;
	size_t offset = 0;

	uint16_t width = WADReader::ReadUInt16(data, offset);
	uint16_t height = WADReader::ReadUInt16(data, offset);

	PatchData patch;
	patch.XOffset = WADReader::ReadInt16(data, offset);
	patch.YOffset = WADReader::ReadInt16(data, offset);
	patch.PixelData = GenImageColor(width, height, BLANK);

	auto& palette = SourceWad.PalettesLump->Contents[0];

	std::vector<uint32_t> colOffsets;
	for (uint16_t x = 0; x < width; x++)
	{
		colOffsets.push_back(WADReader::ReadUInt(data, offset));
	}

	for (uint16_t x = 0; x < width; x++)
	{
		size_t colOffset = colOffsets[x];

		size_t postOffet = colOffset;

		uint8_t yOffset = WADReader::ReadUInt8(data, postOffet);
		if (yOffset != 255)
		{
			uint8_t lenght = WADReader::ReadUInt8(data, postOffet);
			uint8_t pad = WADReader::ReadUInt8(data, postOffet);

			for (uint8_t y = 0; y <= lenght; y++)
			{
				uint8_t pixelIndex = WADReader::ReadUInt8(data, postOffet);

				Color imageColr = palette.Entry[pixelIndex];

				ImageDrawPixel(&patch.PixelData, x, y+yOffset, imageColr);
			}

			pad = WADReader::ReadUInt8(data, postOffet);
		}
	}

	SourceWad.Patches[patchName] = patch;
}


WADData::TexturesLump::TextureDef* WADFile::LevelMap::FindTexture(const std::string& name)
{
	for (auto textureGroupItr = SourceWad.TextureLumps.rbegin(); textureGroupItr != SourceWad.TextureLumps.rend(); textureGroupItr++)
	{
		auto textureItr = (*textureGroupItr)->Contents.find(name);
		if (textureItr != (*textureGroupItr)->Contents.end())
		{
			return &textureItr->second;
		}
	}

	return nullptr;
}

void WADFile::LevelMap::CacheTexture(const std::string& textureName)
{
	if (SourceWad.Textures.find(textureName) != SourceWad.Textures.end())
		return;
	
	auto* textureDef = FindTexture(textureName);
	if (!textureDef)
		return;

	Image textureImage = GenImageColor(textureDef->Width, textureDef->Height, BLANK);

	for (const auto& patch : textureDef->Patches)
	{
		const std::string& patchName = SourceWad.PatchNames->Contents[patch.PatchId];

		CachePatch(patchName);

		auto patchItr = SourceWad.Patches.find(patchName);
		if (patchItr == SourceWad.Patches.end())
			continue;
		Rectangle source = { 0, 0, float(patchItr->second.PixelData.width),  float(patchItr->second.PixelData.height) };
		Rectangle destination = { float(patch.OriginX), float(patch.OriginY), source.width, source.height };
		ImageDraw(&textureImage, patchItr->second.PixelData, source, destination, WHITE);
	}

	SourceWad.Textures[textureName] = textureImage;
}

float GetLightFactor(const Vector2& normal)
{
	static Vector2 LightDir = Vector2Normalize(Vector2{ 1,1 });

	float dot = Vector2DotProduct(normal, LightDir);

	if (dot < 0)
		return 0.35f;

	return 0.35f + (dot * 0.75f);
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

	for (const auto& side : Sides->Contents)
	{
		CacheTexture(side.LowerTexture);
		CacheTexture(side.MidTexture);
		CacheTexture(side.TopTexture);
	}

	// cache the edges in a sector
	for (size_t lineIndex = 0; lineIndex < Lines->Contents.size(); lineIndex++)
	{
		auto& line = Lines->Contents[lineIndex];

		auto sp = Verts->Contents[line.Start].Position;
		auto ep = Verts->Contents[line.End].Position;

		if (line.FrontSideDef != WADData::InvalidSideDefIndex)
		{
			auto& side = Sides->Contents[line.FrontSideDef];
			auto& sector = Sectors->Contents[side.SectorId];

			SectorInfo::Edge edge;
			edge.Line = lineIndex;
			edge.Reverse = false;

			edge.Direction = Vector2Normalize(Vector2Subtract(ep, sp));
			edge.Normal = Vector2{ -edge.Direction.y, edge.Direction.x };
			edge.LightFactor = GetLightFactor(edge.Normal);

			edge.Side = line.FrontSideDef;
			if (line.BackSideDef != WADData::InvalidSideDefIndex)
				edge.Destination = Sides->Contents[line.BackSideDef].SectorId;

			SectorCache[side.SectorId].Edges.push_back(edge);
		}

		if (line.BackSideDef != WADData::InvalidSideDefIndex)
		{
			auto& side = Sides->Contents[line.FrontSideDef];
			auto& sector = Sectors->Contents[side.SectorId];

			SectorInfo::Edge edge;
			edge.Line = lineIndex;
			edge.Reverse = true;

			edge.Direction = Vector2Normalize(Vector2Subtract(sp, ep));
			edge.Normal = Vector2{ -edge.Direction.y, edge.Direction.x };
			edge.LightFactor = GetLightFactor(edge.Normal);

			edge.Side = line.BackSideDef;

			if (line.FrontSideDef != WADData::InvalidSideDefIndex)
				edge.Destination = Sides->Contents[line.FrontSideDef].SectorId;

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

	for (auto& thing : Things->Contents)
	{
		thing.SectorId = GetSectorFromPoint(thing.Position.x, thing.Position.y);
	}
//	FindLeafs(Nodes->Contents.size()-1);
}

Vector2 WADFile::LevelMap::GetVertex(size_t index, bool isGLVert) const
{
	if (isGLVert)
		return GLVerts->Contents[index];

	return Verts->Contents[index].Position;
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
