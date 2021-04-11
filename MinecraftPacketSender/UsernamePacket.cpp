#include "UsernamePacket.h"

#define strdup _strdup

UsernamePacket::UsernamePacket(const char* username)
{
    _ID = 0x00;
    _username = strdup(username);
    _stringLength = strlen(_username);
    _totalLength = getLengthInternal() - sizeof(_totalLength);
}

char* UsernamePacket::getBytes()
{
    char* buffer = new char[0xFF];
    int i = 0;
    buffer[i++] = _totalLength;
    buffer[i++] = _ID;
    buffer[i++] = _stringLength;

    int auxIndex = 0;
    while (auxIndex < _stringLength)
    {
        buffer[i++] = _username[auxIndex];
        auxIndex++;
    }

    return buffer;
}

uint8_t UsernamePacket::getLength() 
{
    return _totalLength + sizeof(_totalLength);
}

inline uint8_t UsernamePacket::getLengthInternal()
{
    return  sizeof(_totalLength) + sizeof(_ID) + sizeof(_stringLength) + _stringLength;
}

UsernamePacket::~UsernamePacket() 
{
    delete _username;
}
