#include "Socket.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <thread>
#include "zlib.h"

Socket::Socket(PCSTR server, PCSTR port)
{
    _server = server;
    _port = port;
}

int Socket::getLastError()
{
    return WSAGetLastError();
}

void Socket::sendData(const char* data, int length)
{
    int iResult;
    iResult = send(_socket, data, length, 0);

    if (iResult == SOCKET_ERROR)
    {
        closeConnection();
        return;
    }
}

void Socket::openConnection()
{
    if (isConnected)
        return;

    createSocket();

    isConnected = getLastError() == 0x00;
}

void Socket::closeConnection()
{
    closesocket(_socket);
    WSACleanup();
    isConnected = false;
}

void Socket::startMessageThread()
{
    if (isThreadRunning)
        return;

    std::thread getMessageThread(&Socket::getMessage, this);
    getMessageThread.detach();

    isThreadRunning = true;
}

void Socket::stopMessageThread()
{
    isThreadRunning = false;
}

void Socket::getMessage()
{
    //DEBUG
    int lastMessageIndex = 0;
    char lastPacket[32768];
    //DEBUG

    char buffer[2048];

    int length = 0;
    int bytesRead = 0;

    int messageIndex = 0;
    char* message = new char[32768];

    

    while (isThreadRunning)
    {
        if (bytesRead <= 0)
            bytesRead = recv(_socket, buffer, sizeof(buffer), 0);

        if (bytesRead > sizeof(buffer))
            bytesRead = 0;

        if (bytesRead > 0)
        {
            if (length <= 0)
                length = readVarInt(buffer, &bytesRead);

            int auxLength = length;
            for(int i = 0; i <= auxLength; i++)
            {
                message[messageIndex++] = buffer[i];

                bytesRead--;
                length--;

                if (length <= 0)
                {
                    for (int i = 0; i < messageIndex; i++)
                    {
                        lastPacket[i] = message[i];
                    }

                    lastMessageIndex = messageIndex;

                    parseMessage(message, messageIndex);

                    int auxIndex = 0;
                    for (int i = messageIndex; i < sizeof(buffer); i++)
                    {
                        buffer[auxIndex] = buffer[i];
                        auxIndex++;
                    }

                    messageIndex = 0;

                    break;
                }

                if (bytesRead == 0)
                    break;
            }
        }
        
        Sleep(30);
    }
}

void Socket::parseMessage(char* packet, int lenght) 
{
    int packetId = 0;
    bool isThisPacketCompressed = _isCompressedSet;

    if(_isCompressedSet)
    {
        int dataLength = readVarInt(packet, &lenght);
        if (dataLength == 0) 
        {
            isThisPacketCompressed = false;
            packetId = readVarInt(packet, &lenght);
        }
        else 
        {
            //char nome[6] = "Edson";
            //char* buff = new char[100];

            //z_stream infstream;
            //infstream.zalloc = Z_NULL;
            //infstream.zfree = Z_NULL;
            //infstream.opaque = Z_NULL;

            //// setup "b" as the input and "c" as the compressed output
            //infstream.avail_in = 5; // size of input
            //infstream.next_in = (Bytef*)nome; // input char array
            //infstream.avail_out = 100; // size of output
            //infstream.next_out = (Bytef*)buff; // output char array

            //// the actual DE-compression work.
            //deflateInit(&infstream, Z_BEST_COMPRESSION);
            //deflate(&infstream, Z_FINISH);
            //deflateEnd(&infstream);

            char* decompressedPacket = decompressMessage(packet, lenght);
            //dataLength++;
            //packetId = readVarInt(decompressedPacket, &dataLength);
        }
    }
    else 
    {
        packetId = readVarInt(packet, &lenght);
    }
        

    switch (packetId)
    {
    case 0x01:
        //join game
        std::cout << "join";
        break;
    case 0x02:
        //login sucess
        std::cout << "login";
        break;
    case 0x03:
        if (!_isCompressedSet)
        {
            //setting compress
            _isCompressedSet = true;
            _maxCompressionLength = readVarInt(packet, &lenght);
        }
        break;
    case 0x05:
        std::cout << "spawn location";
        break;
    case 0x08:
        std::cout << "location";
        break;
    default:
        break;
    }
}

char* Socket::decompressMessage(char* compressedPacket, int length)
{
    char* decompressedMessagePtr = new char[0x10000];

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    stream.avail_in = length;
    stream.next_in = (Bytef*)compressedPacket;
    stream.avail_out = 0x10000;
    stream.next_out = (Bytef*)decompressedMessagePtr;

    // the actual DE-compression work.
    inflateInit(&stream);
    inflate(&stream, Z_NO_FLUSH);
    inflateEnd(&stream);
    
    char z[10000];//= readVarInt(decompressedMessagePtr, &length);

    for (int i = 0; i < 10000; i++)
    {
        z[i] = decompressedMessagePtr[i];
    }

    return decompressedMessagePtr;
}

int Socket::readVarInt(char* packet, int* length)
{
    return (uint32_t)internalRead(packet, length, 5);
}

long Socket::readVarLong(char* packet, int* length)
{
    return internalRead(packet, length, 10);
}

long Socket::internalRead(char* packet, int* length, int8_t maxNumRead)
{
    int numRead = 0;
    long result = 0;
    char read;

    while (1)
    {
        read = packet[numRead];
        int value = (read & 0b01111111);
        result |= (value << (7 * numRead));
        numRead++;

        auto bitWase = (read & 0b10000000);
        if (bitWase == 0)
            break;

        if (numRead > maxNumRead)
            return 0xFFFFFFFF;
    };

    int auxIndex = 0;
    for (int i = numRead; i < *length; i++)
    {
        packet[auxIndex] = packet[i];
        auxIndex++;
    }

    *length = *length - numRead;
    return result;
}

void Socket::createSocket()
{
    SOCKET ConnectSocket = INVALID_SOCKET;

    WSADATA wsaData;

    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        return;
    }

    addrinfo* result = NULL;
    addrinfo* ptr = NULL;
    addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(_server, _port, &hints, &result);
    if (iResult != 0)
    {
        WSACleanup();
        return;
    }

    ptr = result;

    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET)
    {
        freeaddrinfo(result);
        WSACleanup();
        return;
    }

    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        return;
    }

    freeaddrinfo(result);

    _socket = ConnectSocket;
}

Socket::~Socket() 
{
    delete _server;
    delete _port;
}
