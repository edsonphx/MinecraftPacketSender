#include "StatusPacket.h"

uint8_t StatusPacket::getLength()
{
    return sizeof(_data);
}

const char* StatusPacket::getBytes()
{
    return (const char*)&_data;
}
