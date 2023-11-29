#include "lump_types.h"

#include "reader.h"

namespace WADData
{
    Lump* GetLump(const std::string& name)
    {
		// map lumps
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
		if (name == SEGS)
			return new SegsLump();
		if (name == SSECTORS)
			return new SubSectorsLump();
		if (name == NODES)
			return new NodesLump();

		// map gl lumps
		if (name == GL_VERT)
			return new GLVertsLump();
		if (name == GL_SEGS)
			return new GLSegsLump();
        if (name == GL_SSECT)
            return new GLSubSectorsLump();


		// texture lumps
		if (name == PLAYPAL)
			return new PlayPalLump();


        return nullptr;
    }

    void ThingsLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion)
    {
        size_t count = size / Thing::ReadSize;

        Contents.resize(count);
        for (size_t i = 0; i < count; i++)
        {
            size_t readOffset = offset;

			Contents[i].X = WADReader::ReadInt16(data, readOffset);
			Contents[i].Y = WADReader::ReadInt16(data, readOffset);
			Contents[i].BinAngle = WADReader::ReadInt16(data, readOffset);
			Contents[i].TypeId = WADReader::ReadUInt16(data, readOffset);
			Contents[i].Flags = WADReader::ReadUInt16(data, readOffset);
	
			Contents[i].Position = Vector2{ Contents[i].X * MapScale, Contents[i].Y * MapScale };
			Contents[i].Angle = float(Contents[i].BinAngle);

			offset += Thing::ReadSize;
        }
    }

    void VertexesLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion)
    {
        size_t count = size / Vertex::ReadSize;

        Contents.resize(count);
        for (size_t i = 0; i < count; i++)
        {
            memcpy(&Contents[i], data + offset, Vertex::ReadSize);
            offset += Vertex::ReadSize;
        }
    }

    void LineDefLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion)
    {
        size_t count = size / LineDef::ReadSize;

        Contents.resize(count);
        for (size_t i = 0; i < count; i++)
        {
            memcpy(&Contents[i], data + offset, LineDef::ReadSize);
            offset += LineDef::ReadSize;
        }
    }

	void SideDefLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion)
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

			Contents[i].Offset = Vector2{ Contents[i].XOffset * MapScale, Contents[i].XOffset * MapScale };

			offset += SideDef::ReadSize;
		}
	}

	void SectorsLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion)
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

			Contents[i].Floor = Contents[i].FloorHeight * MapScale;
			Contents[i].Ceiling = Contents[i].CeilingHeight * MapScale;

			offset += Sector::ReadSize;
		}
	}

	void SegsLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion)
	{
		size_t count = size / Seg::ReadSize;

		Contents.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			memcpy(&Contents[i], data + offset, Seg::ReadSize);
			offset += Seg::ReadSize;
		}
	}

	void SubSectorsLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion)
	{
		size_t count = size / SubSector::ReadSize;

		Contents.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			memcpy(&Contents[i], data + offset, SubSector::ReadSize);
			offset += SubSector::ReadSize;
		}
	}

	void NodesLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion)
	{
		size_t count = size / Node::ReadSize;

		Contents.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			memcpy(&Contents[i], data + offset, Node::ReadSize);
			offset += Node::ReadSize;
		}
	}

	void GLVertsLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion)
	{
		if (size < 4)
			return;

		std::string magic = "XXXX";
		memcpy((char*)magic.c_str(), data + offset, 4);

		if (magic == "gNd2")
			FormatVersion = 2;
		else if (magic == "gND5")
			FormatVersion = 5;

		bool wideVerts = FormatVersion != 0;

		size_t readSize = 4;
		if (wideVerts)
		{
			readSize = 8;
			size -= 4;
			offset += 4;
		}

		size_t count = size / readSize;

		Contents.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			if (wideVerts)
			{
				int32_t x;
				memcpy(&x, data + offset, 4);

				int32_t y;
				memcpy(&y, data + offset + 4, 4);

				Contents[i].x = x / 65536.0f;
				Contents[i].y = y / 65536.0f;
			}
			else
			{
				int16_t x;
				int16_t y;

				memcpy(&x, data + offset, 2);
				memcpy(&y, data + offset + 2, 2);

				Contents[i].x = x;
				Contents[i].y = y;
			}
			
			Contents[i].x *= MapScale;
			Contents[i].y *= MapScale;

			offset += readSize;
		}
	}

	void GLSegsLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion /*= 0*/)
	{
		std::string magic = "XXXX";
		memcpy((char*)magic.c_str(), data + offset, 4);

		if (magic == "gNd3")
			FormatVersion = 3;
		else if (glVertsVersion == 5)
			FormatVersion = 5;

		size_t readSize = 10;
		if (FormatVersion != 0)
		{
			readSize = 16;

			if (FormatVersion == 3)
			{
				size -= 4;
				offset += 4;
			}
		}

		size_t count = size / readSize;

		Contents.resize(count);

		uint8_t glOffset = 30;
		if (FormatVersion == 5)
			glOffset = 31;

		for (size_t i = 0; i < count; i++)
		{
			size_t readOffset = offset;

			auto& seg = Contents[i];

			if (FormatVersion != 0)
			{
				seg.Start = WADReader::ReadUInt(data, readOffset);
				if (uint32_t(seg.Start) & 1 << glOffset)
				{
					seg.SartIsGL = true;
					seg.Start &= ~(1 << glOffset);
				}

				seg.End = WADReader::ReadUInt(data, readOffset);
				if (uint32_t(seg.End) & 1 << glOffset)
				{
					seg.EndIsGL = true;
					seg.End &= ~(1 << glOffset);
				}

				seg.LineIndex = WADReader::ReadUInt16(data, readOffset);
				seg.Direction = WADReader::ReadUInt16(data, readOffset);
				seg.PartnerSegIndex = WADReader::ReadUInt(data, readOffset);
			}
			else
			{
				seg.Start = WADReader::ReadUInt16(data, readOffset);
				if (seg.Start & 1 << 15)
				{
					seg.SartIsGL = true;
					seg.Start &= ~(1 << 15);
				}

				seg.End = WADReader::ReadUInt16(data, readOffset);
				if (seg.End & 1 << 15)
				{
					seg.EndIsGL = true;
					seg.End &= ~(1 << 15);
				}

				seg.LineIndex = WADReader::ReadUInt16(data, readOffset);
				seg.Direction = WADReader::ReadUInt16(data, readOffset);
				seg.PartnerSegIndex = WADReader::ReadUInt16(data, readOffset);
			}

			offset += readSize;
		}
	}

    void GLSubSectorsLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion /*= 0*/)
    {
        std::string magic = "XXXX";
        memcpy((char*)magic.c_str(), data + offset, 4);

        if (magic == "gNd3")
            FormatVersion = 3;
        else if (glVertsVersion == 5)
            FormatVersion = 5;

        size_t readSize = 4;
        if (FormatVersion != 0)
        {
            readSize = 8;

            if (FormatVersion == 3)
            {
                size -= 4;
                offset += 4;
            }
        }

        size_t count = size / readSize;

        Contents.resize(count);

		for (size_t i = 0; i < count; i++)
		{
			size_t readOffset = offset;
			auto& subsector = Contents[i];

			if (FormatVersion > 0)
			{
                subsector.Count = WADReader::ReadUInt(data, readOffset);
                subsector.StartSegment = WADReader::ReadUInt(data, readOffset);
			}
			else
			{
                subsector.Count = WADReader::ReadUInt16(data, readOffset);
                subsector.StartSegment = WADReader::ReadUInt16(data, readOffset);
			}

			offset += readSize;
		}
    }

	void PlayPalLump::Parse(uint8_t* data, size_t offset, size_t size, int glVertsVersion /*= 0*/)
	{
		size_t count = size / Palette::ReadSize;

		for (size_t i = 0; i < count; i++)
		{
			size_t readOffset = offset;
			for (int entryIndex = 0; entryIndex < 256; entryIndex++)
			{
				Color c = { 0,0,0,255 };
				c.r = *(data + readOffset++);
				c.g = *(data + readOffset++);
				c.b = *(data + readOffset++);
				Contents[i].Entry.push_back(c);
			}

			offset += Palette::ReadSize;
		}
	}

}