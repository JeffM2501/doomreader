#pragma once

#include <vector>
#include <string>
#include <functional>

#include "raylib.h"

namespace WADData
{
    class Lump;

    struct DirectoryEntry
    {
        size_t LumpOffset = 0;
        size_t LumpSize = 0;
        std::string Name;

        uint8_t* BufferData = nullptr;
    };

    class Lump
    {
    public:
        virtual void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) {}

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

    static constexpr char GL_VERT[]     = "GL_VERT";
    static constexpr char GL_SEGS[]     = "GL_SEGS";
    static constexpr char GL_SSECT[]    = "GL_SSECT";
    static constexpr char GL_NODES[]    = "GL_NODES";
    static constexpr char GL_PVS[]      = "GL_PVS";

    static constexpr char PLAYPAL[] = "PLAYPAL";

    static constexpr float MapScale = 1.0f / 32.0f;

    Lump* GetLump(const std::string& name);

    class ThingsLump : public Lump
    {
    public:
        void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

        struct Thing
        {
            int16_t X = 0;
            int16_t Y = 0;
            int16_t BinAngle = 0;
            uint16_t TypeId = 0;
            uint16_t Flags = 0;

            Vector2 Position = { 0,0 };
            float Angle = 0;



            static constexpr size_t ReadSize = 10;
        };

        std::vector<Thing> Contents;


        std::unordered_map<uint16_t, std::vector<Thing*>> ThingsByType;
    };

    class VertexesLump : public Lump
    {
    public:
        void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

        struct Vertex
        {
            int16_t X = 0;
            int16_t Y = 0;

            Vector2 Position = { 0 };
            static constexpr size_t ReadSize = 4;
        };

        std::vector<Vertex> Contents;
    };

	static constexpr uint16_t InvalidSectorIndex = uint16_t(-1);
	static constexpr uint16_t InvalidSideDefIndex = uint16_t(-1);

    class LineDefLump : public Lump
    {
    public:
        void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

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
		void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

		struct SideDef
		{
			int16_t XOffset = 0;
			int16_t YOffset = 0;
            std::string TopTexture;
            std::string MidTexture;
            std::string LowerTexture;
			uint16_t SectorId = InvalidSectorIndex;

            Vector2 Offset = { 0 };

			static constexpr size_t ReadSize = 30;
		};

		std::vector<SideDef> Contents;
	};
	
    class SectorsLump : public Lump
    {
    public:
        void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

		struct Sector
		{
			int16_t FloorHeight = 0;
           
			int16_t CeilingHeight = 0;
            std::string FloorTexture;
            std::string CeilingTexture;
			int16_t LightLevel = 0;
			uint16_t SpecialType = 0;
			uint16_t TagNumber = 0;

            float Floor = 0;
            float Ceiling = 0;

			static constexpr size_t ReadSize = 26;
		};

        std::vector<Sector> Contents;
    };

	class SegsLump : public Lump
	{
	public:
		void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

		struct Seg
		{
			uint16_t Start = 0;
			uint16_t End = 0;
			int16_t Angle = 0;
			uint16_t LineIndex = 0;
			uint16_t Direction = 0;
            int16_t Offset = 0;

			static constexpr size_t ReadSize = 12;
		};

		std::vector<Seg> Contents;
	};

    class SubSectorsLump : public Lump
	{
	public:
		void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

		struct SubSector
		{
			uint16_t Count = 0;
			uint16_t StartIndex = 0;

			static constexpr size_t ReadSize = 4;
		};

		std::vector<SubSector> Contents;
    };

	class NodesLump : public Lump
	{
	public:
		void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

		struct Node
		{
            Vector2 PartitionStart = { 0 };
            Vector2 PartitionVector = { 0 };

            Rectangle RightBounds = { 0 };
            Rectangle LeftBounds = { 0 };

            int16_t PartitionStartX;
            int16_t PartitionStartY;
			int16_t PartitionSlopeX;
			int16_t PartitionSlopeY;
            int16_t RightBBox[4];
            int16_t LeftBBox[4];

			uint16_t RightChild;
			uint16_t LeftChild;

			static constexpr size_t ReadSize = 28;
		};

		std::vector<Node> Contents;
	};

	class GLVertsLump : public Lump
	{
	public:
		void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

        int FormatVersion = 0;

		std::vector<Vector2> Contents;
	};

	class GLSegsLump : public Lump
	{
	public:
		void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

        struct GLSeg
        {
			size_t Start = 0;
            bool SartIsGL = false;
			size_t End = 0;
            bool EndIsGL = false;

			size_t LineIndex = 0;
			size_t Direction = 0;
			size_t PartnerSegIndex = 0;
        };

		int FormatVersion = 0;

		std::vector<GLSeg> Contents;
	};

	class GLSubSectorsLump : public Lump
	{
	public:
		void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

		struct GLSubSector
		{
            size_t Count = 0;
            size_t StartSegment = 0;
		};

		int FormatVersion = 0;

		std::vector<GLSubSector> Contents;
	};


    class PlayPalLump : public Lump
    {
	public:
		void Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion = 0) override;

		struct Palette
		{
            std::vector<Color> Entry;

            static constexpr size_t ReadSize = 256 * 3;
		};

		std::unordered_map<size_t, Palette> Contents;
    };

}