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

        virtual ~Lump() = default;

        std::function<void(Lump*)> Visualize = nullptr;
    };

    static constexpr char THINGS[]      = "THINGS";
    static constexpr char LINEDEFS[]    = "LINEDEFS";
    static constexpr char SIDEDEFS[]    = "SIDEDEFS";
    static constexpr char VERTEXES[]    = "VERTEXES";
    static constexpr char SEGS[]        = "SEGS";
    static constexpr char SSECTORS[]    = "SSECTORS";
    static constexpr char NODES[]       = "NODES";
    static constexpr char SECTORS[]     = "SECTORS";
    static constexpr char REJECT[]      = "REJECT";
    static constexpr char BLOCKMAP[]    = "BLOCKMAP";

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

	static constexpr uint16_t InvalidSectorIndex = uint16_t(-1);
	static constexpr uint16_t InvalidSideDefIndex = uint16_t(-1);

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
            uint16_t Sector = InvalidSectorIndex;
            uint16_t FrontSideDef = InvalidSideDefIndex;
            uint16_t BackSideDef = InvalidSideDefIndex;

            static constexpr size_t ReadSize = 14;
        };

        std::vector<LineDef> Contents;
    };

	class SideDefLump : public Lump
	{
	public:
		void Parse(uint8_t* data, size_t offset, size_t size) override;

		struct SideDef
		{
			int16_t XOffset = 0;
			int16_t YOffset = 0;
            std::string TopTexture;
            std::string MidTexture;
            std::string LowerTexture;
			uint16_t SectorId = InvalidSectorIndex;

			static constexpr size_t ReadSize = 30;
		};

		std::vector<SideDef> Contents;
	};

	
    class SectorsLump : public Lump
    {
    public:
        void Parse(uint8_t* data, size_t offset, size_t size) override;

		struct Sector
		{
			int16_t FloorHeight = 0;
			int16_t CeilingHeight = 0;
            std::string FloorTexture;
            std::string CeilingTexture;
			int16_t LightLevel = 0;
			uint16_t SpecialType = 0;
			uint16_t TagNumber = 0;

			static constexpr size_t ReadSize = 26;
		};

        std::vector<Sector> Contents;
    };
}