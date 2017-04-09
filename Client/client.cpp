#include "client.h"

#include <stdio.h>
#include <winsock2.h>
#include <errno.h>
#include <thread>
#include <QDebug>
#include "connection.h"
#include "packet.h"

#include <ws2tcpip.h>

#include <iostream>
#include <fstream>


Client::Client(QObject *parent) : QObject(parent)
{

}


Connection conn;
SOCKET socket_tcp;
SOCKET socket_udp;
sockaddr_in server;



void Client::start(){
    std::thread(&Client::startTCP, this).detach();
    std::thread(&Client::startUDP, this).detach();
}


void Client::startTCP(){
    if(!conn.WSAStartup())
        return;
    connectTCP();
    WSACleanup();
    closesocket (socket_tcp);
    qDebug() << "Client::startTCP() Socket " << socket_tcp << " closed";
}


void Client::startUDP(){
    if(!conn.WSAStartup())
        return;
    connectUDP();
    WSACleanup();
    closesocket (socket_udp);
    qDebug() << "Client::startUDP() Socket " << socket_udp << " closed";
}





void Client::connectTCP(){
    string host = "localhost";
    int port =	7000;

    if(!conn.WSASocketTCP(socket_tcp))
        return;
    if(!conn.setoptSO_REUSEADDR(socket_tcp))
        return;
    if(!conn.connect(socket_tcp, host, port))
        return;
    runTCP();
}


void Client::connectUDP(){
    string host = "localhost";
    int port =	7000;

    if(!conn.WSASocketUDP(socket_udp))
        return;
    if(!conn.setoptSO_REUSEADDR(socket_udp))
        return;
    if(!conn.bind(socket_udp, server, port))
        return;
    if(!conn.setoptIP_ADD_MEMBERSHIP(socket_udp))
        return;
    runUDP();
}


void Client::runTCP(){
    char rbuf[BUFFERSIZE];
    WSAEVENT readEvent;

    if(!conn.WSACreateEvent(readEvent))
        return;
    if(!conn.WSAEventSelect(socket_tcp, readEvent, FD_READ))
        return;

    while(true) {
        if(!conn.WSAWaitForMultipleEvents(readEvent))
            return;
        WSAResetEvent(readEvent);

        if(!conn.recv(socket_tcp, rbuf))
            continue;

        //Handle received songs / lists here.
    }
}



void Client::runUDP(){
    char rbuf[BUFFERSIZE];
    WSAEVENT readEvent;

    if(!conn.WSACreateEvent(readEvent))
        return;
    if(!conn.WSAEventSelect(socket_udp, readEvent, FD_READ))
        return;
    while (true) {
        if(!conn.WSAWaitForMultipleEvents(readEvent))
            return;
        WSAResetEvent(readEvent);

        int n = conn.recv(socket_udp, rbuf);
        if(n<=0)
            continue;

        if(rbuf[0] == '0'){
            emit receivedHeader(&rbuf[1], n-1);
        }else if(rbuf[0] == '1'){
            emit receivedChunkData(&rbuf[1], n-1);
        }
    }
}


void Client::requestSong(QString song){
    char buffer[BUFFERSIZE];

    QString packet;
    packet.append(DOWNLOAD);
    packet.append(song);

    memset((char *)buffer, 0, BUFFERSIZE);
    memcpy(buffer, packet.toStdString().c_str(), BUFFERSIZE);

    if(!conn.send(socket_tcp, buffer))
        return;
}





