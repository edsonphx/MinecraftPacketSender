#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdint>
#include <thread>

class Socket
{
public:
    bool isConnected = false;
    bool isThreadRunning = false;

    Socket(PCSTR server, PCSTR port);

    int getLastError();

    void sendData(const char* data, int length);

    void openConnection();

    void closeConnection();

    void startMessageThread();

    void stopMessageThread();

    ~Socket();

private:
    PCSTR _server;
    PCSTR _port;
    SOCKET _socket;
    bool _isCompressedSet; 
    int _maxCompressionLength; //talvez seja opcional

    void getMessage();

    void createSocket();

    int readVarInt(char* packet, int* length);
    long readVarLong(char* packet, int* length);
    long internalRead(char* packet, int* length, int8_t maxNumRead);

    void parseMessage(char* packet, int length);
    char* decompressMessage(char* compressedPacket, int length);
};

