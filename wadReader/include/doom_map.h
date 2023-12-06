#pragma once

#include "lump_types.h"
#include "raylib.h"

#include <string>
#include <set>
#include <vector>
#include <unordered_map>

#include "reader.h"

class WADFile
{
public:

    uint8_t* BufferData = nullptr;

    std::unordered_map<std::string, WADData::DirectoryEntry> Entries;

	class LumpDatabase
	{
	public:
		template<class T>
		T* GetLump(const std::string& name)
		{
			auto itr = Lumps.find(name);
			if (itr == Lumps.end())
				return nullptr;

			return (T*)(itr->second);
		}

		void LoadLumpData(const WADData::DirectoryEntry& entry);

	protected:
		std::unordered_map<std::string, WADData::Lump*> Lumps;
	};

	LumpDatabase LumpDB;

    virtual ~WADFile()
    {
        if (BufferData)
            UnloadFileData(BufferData);
    }

	virtual void Read(const char* fileName);

	WADData::PlayPalLump* PalettesLump = nullptr;

	WADData::PatchNamesLump* PatchNames = nullptr;

	std::vector<WADData::TexturesLump*> TextureLumps;

	class LevelMap
	{
	public:
		LevelMap(WADFile& source) : SourceWad(source) {};

		WADFile& SourceWad;
		std::string Name;
		std::unordered_map<std::string, WADData::DirectoryEntry> Entries;
		LumpDatabase LumpDB;

		WADData::VertexesLump* Verts = nullptr;
		WADData::LineDefLump* Lines = nullptr;
		WADData::ThingsLump* Things = nullptr;
		WADData::SectorsLump* Sectors = nullptr;
		WADData::SideDefLump* Sides = nullptr;
		WADData::SegsLump* Segs = nullptr;
		WADData::SubSectorsLump* Subsectors = nullptr;
		WADData::NodesLump* Nodes = nullptr;

		WADData::GLVertsLump* GLVerts = nullptr;
		WADData::GLSegsLump* GLSegs = nullptr;
		WADData::GLSubSectorsLump* GLSubSectors = nullptr;

		struct SectorInfo
		{
			struct Edge
			{
				// The line for this edge
				size_t Line = 0;
				bool Reverse = false;

				// The side that we would draw if we are inside this sector
				size_t Side = 0;

				// The sector on the other side of this line
				size_t Destination = WADData::InvalidSideDefIndex;

				Vector2 Direction = { 0 };
				Vector2 Normal = { 0 };

				float LightFactor = 0.25f;
			};

			// The sorted list of edges for this sector (forms a loop)
			std::vector<Edge> Edges;

			std::vector<size_t>  SubSectors;

			Color Tint = WHITE;

			size_t SectorIndex = 0;
		};

		std::vector<SectorInfo> SectorCache;

		std::set<size_t> LeafNodes;

		void Load();

		Vector2 GetVertex(size_t index, bool isGLVert) const;

		size_t GetSectorFromPoint(float x, float y, size_t* subSector = nullptr) const;

		WADData::TexturesLump::TextureDef* FindTexture(const std::string& name);

	protected:
		void FindLeafs(size_t node);

		void CacheFlat(const std::string& flatName);
		void CachePatch(const std::string& patchName);
		void CacheTexture(const std::string& textureName);
	};

	struct PatchData
	{
		int XOffset = 0;
		int YOffset = 0;
		Image PixelData = { 0 };
	};

	std::vector<LevelMap> Levels;

	std::unordered_map<std::string, Image> Flats;
	std::unordered_map<std::string, PatchData> Patches;
	std::unordered_map<std::string, Image> Textures;

};