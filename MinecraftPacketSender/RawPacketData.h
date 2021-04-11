#pragma once

class RawPacketData
{
public:

    RawPacketData(char* bytes, int length);
    char* getBytes();
    int getLength();
    ~RawPacketData();
private:
    char* _bytes;
    int _length;
};

