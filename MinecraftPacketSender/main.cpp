#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "HandShakePacket.h"
#include "UsernamePacket.h"
#include "Socket.h"
#include "StatusPacket.h"

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    HandShakePacket handshake = HandShakePacket("52.15.44.224", 25565, 0x2F, 0x02);

    UsernamePacket statusPackage = UsernamePacket("Edson2021");

    Socket socket = Socket("52.15.44.224", "25565");
    socket.openConnection();
    
    socket.sendData(handshake.getBytes(), handshake.getLength());
    Sleep(100);
    socket.sendData(statusPackage.getBytes(), statusPackage.getLength());
    socket.startMessageThread();
    
    Sleep(100000);
    

    /*PacketParser parser;

    auto newPacket = parser.CreatePacketUncompressed(packet);*/
}