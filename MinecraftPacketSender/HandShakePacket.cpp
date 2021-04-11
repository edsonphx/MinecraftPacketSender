#include "HandShakePacket.h"
#include <string.h>

#define strdup _strdup

HandShakePacket::HandShakePacket(const char* server, uint16_t port, uint8_t protocol, uint8_t nextInstruction)
{
    _server = strdup(server);
    _port = port;
    _ID = 0x00;
    _protocol = protocol;
    _stringLength = strlen(_server);
    _nextInstruction = nextInstruction;
    _totalLength = getLengthInternal() - sizeof(_totalLength);
}

char* HandShakePacket::getBytes()
{
    char* buffer = new char[0xFF];
    int8_t i = 0;

    buffer[i++] = _totalLength;
    buffer[i++] = _ID;
    buffer[i++] = _protocol;
    buffer[i++] = _stringLength;

    int auxIndex = 0;
    while (auxIndex < _stringLength)
    {
        buffer[i++] = _server[auxIndex];
        auxIndex++;
    }

    char* port = (char*)&_port;
    buffer[i++] = port[1];
    buffer[i++] = port[0];

    buffer[i++] = _nextInstruction;

    return buffer;
}

int16_t HandShakePacket::getLength()
{
    return _totalLength + sizeof(_totalLength);
}

uint8_t HandShakePacket::getLengthInternal()
{
    uint8_t totalLengthSize = sizeof(_totalLength);
    uint8_t idFieldSize = sizeof(_ID);
    uint8_t protocolFieldSize = sizeof(_protocol);
    uint8_t stringLengthSize = sizeof(_stringLength);
    uint8_t portFieldSize = sizeof(_port);
    uint8_t nextInstructionFieldSize = sizeof(_nextInstruction);

    uint8_t serverFieldDataSize = _stringLength;

    uint8_t total = totalLengthSize +
        idFieldSize +
        protocolFieldSize +
        stringLengthSize +
        portFieldSize +
        nextInstructionFieldSize +
        serverFieldDataSize;

    return total;
}

HandShakePacket::~HandShakePacket()
{
    delete _server;
}
