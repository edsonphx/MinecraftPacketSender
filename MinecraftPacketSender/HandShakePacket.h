#pragma once
#include <cstdint>

class HandShakePacket
{
public:
    HandShakePacket(const char* server, uint16_t port, uint8_t protocol, uint8_t nextInstruction);
	char* getBytes();
    int16_t getLength();
    ~HandShakePacket();
private:
    uint8_t getLengthInternal();
    uint8_t _totalLength;
    uint8_t _ID;
    uint8_t _protocol;
    uint8_t _stringLength;
    char* _server;
    uint16_t _port;
    uint8_t _nextInstruction;
};

