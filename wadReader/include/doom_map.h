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
	std::unordered_map<std::string, WADData::Lump*> LumpDB;

    virtual ~WADFile()
    {
        if (BufferData)
            UnloadFileData(BufferData);
    }

	virtual void Read(const char* fileName);

	void LoadLumpData(const WADData::DirectoryEntry& entry);

	template<class T>
	T* GetLump(const std::string& name)
	{
		auto itr = LumpDB.find(name);
		if (itr == LumpDB.end())
			return nullptr;

		return (T*)(itr->second);
	}

	WADData::PlayPalLump* PalettesLump = nullptr;;

	class LevelMap
	{
	public:
		LevelMap(WADFile& source) : SourceWad(source) {};

		WADFile& SourceWad;
		std::string Name;
		std::unordered_map<std::string, WADData::DirectoryEntry*> Entries;
		
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
			};

			// The sorted list of edges for this sector (forms a loop)
			std::vector<Edge> Edges;

			std::vector<size_t>  SubSectors;

			Color Tint = WHITE;
		};

		std::vector<SectorInfo> SectorCache;

		std::set<size_t> LeafNodes;

		std::unordered_map<std::string, Image> Flats;

		void Load();

		Vector2 GetVertex(size_t index, bool isGLVert) const;

	protected:
		void FindLeafs(size_t node);

		void CacheFlat(const std::string& flatName);
	};

	std::vector<LevelMap> Levels;

};