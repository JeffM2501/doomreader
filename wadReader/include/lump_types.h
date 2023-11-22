#pragma once

#include <vector>
#include <string>
#include <functional>

namespace WADData
{
    class Lump;

    struct DirectoryEntry
    {
        size_t LumpOffset;
        size_t LumpSize;
        std::string Name;
    };

    class Lump
    {
    public:
        virtual void Parse(uint8_t* data, size_t offset, size_t size) = 0;

        std::function<void(Lump*)> Visualize = nullptr;
    };

    static constexpr char THINGS[] = "THINGS";
    static constexpr char LINEDEFS[] = "LINEDEFS";
    static constexpr char SIDEDEFS[] = "SIDEDEFS";
    static constexpr char VERTEXES[] = "VERTEXES";
    static constexpr char SEGS[] = "SEGS";
    static constexpr char SSECTORS[] = "SSECTORS";
    static constexpr char NODES[] = "NODES";
    static constexpr char SECTORS[] = "SECTORS";
    static constexpr char REJECT[] = "REJECT";
    static constexpr char BLOCKMAP[] = "BLOCKMAP";

    Lump* GetLump(const std::string& name);

    class ThingsLump : public Lump
    {
    public:
        void Parse(uint8_t* data, size_t offset, size_t size) override;

        struct Thing
        {
            int16_t X = 0;
            int16_t Y = 0;
            int16_t BinAngle = 0;
            uint16_t TypeId = 0;
            uint16_t Flags = 0;

            static constexpr size_t ReadSize = 10;
        };

        std::vector<Thing> Contents;
    };

    class VertexesLump : public Lump
    {
    public:
        void Parse(uint8_t* data, size_t offset, size_t size) override;

        struct Vertex
        {
            int16_t X = 0;
            int16_t Y = 0;
            static constexpr size_t ReadSize = 4;
        };

        std::vector<Vertex> Contents;
    };

    class LineDefLump : public Lump
    {
    public:
        void Parse(uint8_t* data, size_t offset, size_t size) override;

        struct LineDef
        {
            uint16_t Start = 0;
            uint16_t End = 0;
            uint16_t Flags = 0;
            uint16_t SpecialType = 0;
            uint16_t Sector = 0;
            uint16_t FrontSideDef = 0;
            uint16_t BackSideDef = 0;

            static constexpr size_t ReadSize = 14;
        };

        std::vector<LineDef> Contents;
    };

}