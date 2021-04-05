#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <cstdint>

#pragma comment(lib, "Ws2_32.lib")

int getNext(int* i)
{
    (*i)++;
    return *i;
}

struct HandShakePacket
{
public:
    HandShakePacket(const char* server, uint16_t port, uint8_t protocol, uint8_t nextInstruction)
    {
        _server = server;
        _port = port;
        _ID = 0x00;
        _protocol = protocol;
        _stringLenght = strlen(_server);
        _nextInstruction = nextInstruction;
    }
    const char* getBytes()
    {
        char* buffer = new char[0xFF];
        int i = -1;

        buffer[getNext(&i)] = getLenght() - sizeof(_totalLenght);
        buffer[getNext(&i)] = _ID;
        buffer[getNext(&i)] = _protocol;
        buffer[getNext(&i)] = _stringLenght;

        int auxIndex = 0;
        while (auxIndex < _stringLenght)
        {
            buffer[getNext(&i)] = _server[auxIndex];
            auxIndex++;
        }
        
        char* port = (char*)&_port;
        buffer[getNext(&i)] = port[1];
        buffer[getNext(&i)] = port[0];

        buffer[getNext(&i)] = _nextInstruction;

        return (const char*)buffer;
    }

    uint8_t getLenght()
    {
        uint8_t totalLenghtSize = sizeof(_totalLenght);
        uint8_t idFieldSize = sizeof(_ID);
        uint8_t protocolFieldSize = sizeof(_protocol);
        uint8_t stringLenghtSize = sizeof(_stringLenght);
        uint8_t portFieldSize = sizeof(_port);
        uint8_t nextInstructionFieldSize = sizeof(_nextInstruction);

        uint8_t serverFieldDataSize = _stringLenght;

        uint8_t total = totalLenghtSize +
            idFieldSize +
            protocolFieldSize +
            stringLenghtSize +
            portFieldSize +
            nextInstructionFieldSize +
            serverFieldDataSize;

        return total;
    }

private :
    uint8_t _totalLenght;
    uint8_t _ID;
    uint8_t _protocol;
    uint8_t _stringLenght;
    const char* _server;
    uint16_t _port;
    uint8_t _nextInstruction;
};

struct StatusPackage 
{
public:
    const char* getBytes()
    {
        return _data;
    }
    uint8_t getLenght() 
    {
        return sizeof(_data);
    }
private:
    char _data[2] = { 0x01, 0x00 };
};

struct UsernamePacket
{
public:
    UsernamePacket(const char* username)
    {
        _ID = 0x00;
        _username = username;
        _stringLenght = strlen(_username);
        _totalLenght = getLenght() - sizeof(_totalLenght);
    }
    const char* getBytes()
    {
        char* buffer = new char[0xFF];
        int i = -1;
        buffer[getNext(&i)] = _totalLenght;
        buffer[getNext(&i)] = _ID;
        buffer[getNext(&i)] = _stringLenght;

        int auxIndex = 0;
        while (auxIndex < _stringLenght)
        {
            buffer[getNext(&i)] = _username[auxIndex];
            auxIndex++;
        }

        return (const char*)buffer;
    }
    uint8_t getLenght()
    {
        return  sizeof(_totalLenght) + sizeof(_ID) + sizeof(_stringLenght) + _stringLenght;
    }
private:
    uint8_t _totalLenght;
    uint8_t _ID;
    uint8_t _stringLenght;
    const char* _username;
};

struct Socket
{
public:
    int getLastError() 
    {
        return WSAGetLastError();
    }
    Socket(PCSTR server, PCSTR port)
    {
        _server = server;
        _port = port;
    }

    void sendData(const char* data, int lenght) 
    {
        int iResult;
        iResult = send(_socket, data, lenght, 0);

        if (iResult == SOCKET_ERROR) 
        {
            _socket = INVALID_SOCKET;
            return;
        }

        Sleep(100);
    }
    bool openConnection() 
    {
        createSocket();
        return getLastError() == 0x00;
    }
    void closeConnection() 
    {
        closesocket(_socket);
        WSACleanup();
    }
private:
    SOCKET _socket;
    PCSTR _server;
    PCSTR _port;
    int lastError;
    void createSocket()
    {
        lastError = 0x00;
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
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            return;
        }

        freeaddrinfo(result);

        _socket = ConnectSocket;
    }
};


int main()
{
    HandShakePacket request = HandShakePacket("52.15.44.224", 0x63DD, 0x2F, 0x02);

    UsernamePacket statusPackage = UsernamePacket("Edson2021");

    Socket socket = Socket("52.15.44.224", "25565");
    socket.openConnection();

    auto x = request.getBytes();
    auto y = request.getLenght();
    socket.sendData(x, y);
    socket.sendData(statusPackage.getBytes(), statusPackage.getLenght());

    Sleep(10*60000);
}
