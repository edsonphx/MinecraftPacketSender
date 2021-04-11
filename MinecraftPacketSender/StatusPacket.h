#pragma once
#include <cstdint>
#include <WinSock2.h>
class StatusPacket
{
public:
    static const char* getBytes();
    static uint8_t getLength();
private:
    static const short _data = MAKEWORD(0x00,0x01);
};

