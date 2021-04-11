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
#include <vector>
#include <iterator>

Socket::Socket(PCSTR server, PCSTR port)
{
    _server = server;
    _port = port;
    _isCompressedSet = false;
    _maxCompressionLength = 0;
    _socket = 0;
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
    int bufferSize = 2048;
    char* buffer = new char[2048];

    int length = 0;
    int bytesRead = 0;
    
    std::vector<unsigned char>::iterator messageIterator;
    std::vector<unsigned char>* message = new std::vector<unsigned char>();

    while (isThreadRunning)
    {
        if (bytesRead <= 0)
            bytesRead = recv(_socket, buffer, bufferSize, 0);

        if (bytesRead > bufferSize || bytesRead < 0)
            bytesRead = 0;

        if (bytesRead > 0)
        {
            if (length <= 0) 
            {
                length = readVarInt((unsigned char*)buffer, &bytesRead);
                message->resize(length);
                messageIterator = message->begin();
            }

            int expectedConsumedBytes = 1 + length;
            if (length > bytesRead)
                expectedConsumedBytes = bytesRead;

            messageIterator = message->insert(messageIterator, buffer, buffer + expectedConsumedBytes);
            auto msgBody = message->data();

            bytesRead -= expectedConsumedBytes;
            length -= expectedConsumedBytes;

            if (bytesRead <= 0) 
                reajustStartIndex(buffer, bufferSize, expectedConsumedBytes);

            if (length <= 0)
                parseMessage(message);
        }
        
        Sleep(100);
    }
}

void Socket::parseMessage(std::vector<unsigned char>* packet)
{
    int packetId = 0;
    bool isThisPacketCompressed = _isCompressedSet;
    int actualPacketSize = packet->size();

    if(_isCompressedSet)
    {
        int dataLength = readVarInt(packet->data(), &actualPacketSize);
        if (dataLength == 0) 
        {
            isThisPacketCompressed = false;
            packetId = readVarInt(packet->data(), &actualPacketSize);
        }
        else 
        {
            std::vector<unsigned char> messageDecompressed;

            messageDecompressed.resize(dataLength);

            z_stream stream;
            stream.zalloc = Z_NULL;
            stream.zfree = Z_NULL;
            stream.opaque = Z_NULL;

            stream.avail_in = packet->size();
            stream.next_in = packet->data();
            stream.avail_out = messageDecompressed.size();
            stream.next_out = messageDecompressed.data();

            inflateInit(&stream);
            inflate(&stream, Z_NO_FLUSH);
            inflateEnd(&stream);

            packetId = readVarInt(messageDecompressed.data(), &dataLength);
        }
    }
    else 
    {
        packetId = readVarInt(packet->data(), &actualPacketSize);
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
            _maxCompressionLength = readVarInt(packet->data(), &actualPacketSize);
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

int Socket::readVarInt(unsigned char* packet, int* length)
{
    return (uint32_t)internalRead(packet, length, 5);
}

long Socket::readVarLong(unsigned char* packet, int* length)
{
    return internalRead(packet, length, 10);
}

long Socket::internalRead(unsigned char* packet, int* length, int8_t maxNumRead)
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

    reajustStartIndex(packet, *length, numRead);

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

template<typename T>
void Socket::reajustStartIndex(T* arrPtr, int arrSize, int newStartIndex)
{
    int auxIndex = 0;
    for (int i = newStartIndex; i < arrSize - newStartIndex; i++)
    {
        arrPtr[auxIndex++] = arrPtr[i];
    }
}