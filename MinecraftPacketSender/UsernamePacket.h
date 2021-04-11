#pragma once
#include <cstdint>
#include <string.h>

class UsernamePacket
{
public:
    UsernamePacket(const char* username);
    char* getBytes();
    uint8_t getLength();
    ~UsernamePacket();
private:
    uint8_t getLengthInternal();
    uint8_t _totalLength;
    uint8_t _ID;
    uint8_t _stringLength;
    char* _username;
};

