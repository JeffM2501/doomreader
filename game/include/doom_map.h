#pragma once

#include "lump_types.h"
#include "raylib.h"

#include <string>
#include <vector>
#include <unordered_map>

#include "reader.h"

class WADFile
{
public:

    uint8_t* BufferData = nullptr;

    std::vector<WADData::DirectoryEntry> Entries;

    virtual ~WADFile()
    {
        if (BufferData)
            UnloadFileData(BufferData);
    }

	virtual void Read(const char* fileName);

	class LevelMap
	{
	public:
		std::string Name;
		std::vector<WADData::DirectoryEntry> Entries;
		std::unordered_map<std::string, WADData::Lump*> LumpDB;

		WADData::VertexesLump* Verts = nullptr;
		WADData::LineDefLump* Lines = nullptr;
		WADData::ThingsLump* Things = nullptr;
		WADData::SectorsLump* Sectors = nullptr;

		uint8_t* BufferData = nullptr;

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

		void Load();

	protected:
		void LoadLumpData(const WADData::DirectoryEntry& entry);

		template<class T>
		T* GetLump(const std::string& name)
		{
			auto itr = LumpDB.find(name);
			if (itr == LumpDB.end())
				return nullptr;

			return (T*)(itr->second);
		}
	};

	std::vector<LevelMap> Levels;
};