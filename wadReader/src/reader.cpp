#include "reader.h"

namespace WADReader
{
    int32_t ReadInt(uint8_t* buffer, size_t& offset)
    {
        int32_t value = *(int32_t*)(buffer + offset);
        offset += 4;
        return value;
    }

	uint32_t ReadUInt(uint8_t* buffer, size_t& offset)
	{
        uint32_t value = *(uint32_t*)(buffer + offset);
		offset += 4;
		return value;
	}

	int16_t ReadInt16(uint8_t* buffer, size_t& offset)
	{
        int16_t value = *(int16_t*)(buffer + offset);
		offset += 2;
		return value;
	}

	uint16_t ReadUInt16(uint8_t* buffer, size_t& offset)
	{
        uint16_t value = *(uint16_t*)(buffer + offset);
		offset += 2;
		return value;
	}

    std::string ReadName(uint8_t* buffer, size_t& offset)
    {
        char nameBuffer[9] = { 0 };
        memcpy(nameBuffer, buffer + offset, 8);
        offset += 8;
        return std::string(nameBuffer);
    }

    WADData::DirectoryEntry ReadDirectoryEntry(uint8_t* buffer, size_t& readOffset)
    {
        WADData::DirectoryEntry entry;

        entry.LumpOffset = ReadInt(buffer, readOffset);
        entry.LumpSize = ReadInt(buffer, readOffset);

        entry.Name = ReadName(buffer, readOffset);

        return entry;
    }

    std::vector<WADData::DirectoryEntry> ReadDirectoryEntries(uint8_t* buffer)
    {
        size_t offset = 4;

        int32_t lumpCount = ReadInt(buffer, offset);
        size_t dirOffset = ReadInt(buffer, offset);

        std::vector<WADData::DirectoryEntry> entries;

        for (size_t lump = 0; lump < lumpCount; lump++)
        {
            entries.push_back(ReadDirectoryEntry(buffer, dirOffset));
        }

        return entries;
    }
}