#include "lump_types.h"

#include "reader.h"

namespace WADData
{
    Lump* GetLump(const std::string& name)
    {
        if (name == THINGS)
            return new ThingsLump();
        if (name == VERTEXES)
            return new VertexesLump();
        if (name == LINEDEFS)
            return new LineDefLump();
		if (name == SIDEDEFS)
			return new SideDefLump();
		if (name == SECTORS)
			return new SectorsLump();

        return nullptr;
    }

    void ThingsLump::Parse(uint8_t* data, size_t offset, size_t size)
    {
        size_t count = size / Thing::ReadSize;

        Contents.resize(count);
        for (size_t i = 0; i < count; i++)
        {
            memcpy(&Contents[i], data + offset, Thing::ReadSize);
            offset += Thing::ReadSize;
        }
    }

    void VertexesLump::Parse(uint8_t* data, size_t offset, size_t size)
    {
        size_t count = size / Vertex::ReadSize;

        Contents.resize(count);
        for (size_t i = 0; i < count; i++)
        {
            memcpy(&Contents[i], data + offset, Vertex::ReadSize);
            offset += Vertex::ReadSize;
        }
    }

    void LineDefLump::Parse(uint8_t* data, size_t offset, size_t size)
    {
        size_t count = size / LineDef::ReadSize;

        Contents.resize(count);
        for (size_t i = 0; i < count; i++)
        {
            memcpy(&Contents[i], data + offset, LineDef::ReadSize);
            offset += LineDef::ReadSize;
        }
    }

	void SideDefLump::Parse(uint8_t* data, size_t offset, size_t size)
	{
		size_t count = size / SideDef::ReadSize;

		Contents.resize(count);
		for (size_t i = 0; i < count; i++)
		{
            size_t readOffset = offset;

            Contents[i].XOffset = WADReader::ReadInt16(data, readOffset);
            Contents[i].YOffset = WADReader::ReadInt16(data, readOffset);

            Contents[i].TopTexture = WADReader::ReadName(data, readOffset);
            Contents[i].MidTexture = WADReader::ReadName(data, readOffset);
            Contents[i].LowerTexture = WADReader::ReadName(data, readOffset);

            Contents[i].SectorId = WADReader::ReadInt16(data, readOffset);
			offset += SideDef::ReadSize;
		}
	}

	void SectorsLump::Parse(uint8_t* data, size_t offset, size_t size)
	{
		size_t count = size / Sector::ReadSize;

		Contents.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			size_t readOffset = offset;

            uint8_t* p = data + readOffset;

			Contents[i].FloorHeight = WADReader::ReadInt16(data, readOffset);
			Contents[i].CeilingHeight = WADReader::ReadInt16(data, readOffset);

			Contents[i].FloorTexture = WADReader::ReadName(data, readOffset);
			Contents[i].CeilingTexture = WADReader::ReadName(data, readOffset);

			Contents[i].LightLevel = WADReader::ReadInt16(data, readOffset);
            Contents[i].SpecialType = WADReader::ReadInt16(data, readOffset);
            Contents[i].TagNumber = WADReader::ReadInt16(data, readOffset);

			offset += Sector::ReadSize;
		}
	}

}