#include "RawPacketData.h"

RawPacketData::RawPacketData(char* bytes, int length)
{
    _bytes = bytes;
    _length = length;
}

char* RawPacketData::getBytes()
{
    return _bytes;
}

int RawPacketData::getLength()
{
    return _length;
}

RawPacketData::~RawPacketData()
{
    delete _bytes;
}
