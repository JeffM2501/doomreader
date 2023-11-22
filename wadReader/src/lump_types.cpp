#include "lump_types.h"

namespace WADData
{
    Lump* GetLump(const std::string& name)
    {
        if (name == THINGS)
            return new ThingsLump();
        if (name == VERTEXES)
            return new VertexesLump();

        return nullptr;
    }

    void ThingsLump::Parse(uint8_t* data, size_t offset, size_t size)
    {
        size_t count = size / Thing::ReadSize;

        Contents.resize(count);
        for (size_t i = 0; i < count; i++)
        {
            memcpy(&Contents[i], data + offset, Thing::ReadSize);
        }
    }

    void VertexesLump::Parse(uint8_t* data, size_t offset, size_t size)
    {
        size_t count = size / Vertex::ReadSize;

        Contents.resize(count);
        for (size_t i = 0; i < count; i++)
        {
            memcpy(&Contents[i], data + offset, Vertex::ReadSize);
        }
    }
}