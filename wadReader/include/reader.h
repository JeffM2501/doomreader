#pragma once

#include <stdint.h>
#include <string>

#include "lump_types.h"

namespace WADReader
{
    int32_t ReadInt(uint8_t* buffer, size_t& offset);
    uint32_t ReadUInt(uint8_t* buffer, size_t& offset);
    int16_t ReadInt16(uint8_t* buffer, size_t& offset);
    uint16_t ReadUInt16(uint8_t* buffer, size_t& offset);
    uint8_t ReadUInt8(uint8_t* buffer, size_t& offset);
    std::string ReadName(uint8_t* buffer, size_t& offset);

    WADData::DirectoryEntry ReadDirectoryEntry(uint8_t* buffer, size_t& readOffset);
    std::vector<WADData::DirectoryEntry> ReadDirectoryEntries(uint8_t* buffer);
}