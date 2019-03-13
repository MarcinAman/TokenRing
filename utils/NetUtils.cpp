//
// Created by woolfy on 3/9/19.
//

#include <csignal>
#include "NetUtils.h"


int createSocket(Protocol);

int NetUtils::socketForReceiving(Protocol protocol, uint16_t port) {
    int socketFD = createSocket(protocol);

    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; //localhost?
    address.sin_port = htons(port);

    int bindResult = bind(socketFD, (struct sockaddr *)&address, sizeof(address));

    if(bindResult < 0){
        throw std::runtime_error("Failed to connect to receiving socket: " + std::to_string(port) + "\n" + string(strerror(errno)));
    }

    int listenResult = listen(socketFD, 64); // w sumie to cokolwiek wieksze od 1 bo i tak nie powinnismy miec w kolejce wiecej niz 1

    if(listenResult < 0){
        throw std::runtime_error("Failed to listen: " + string(strerror(errno)));
    }

    int addrLen = sizeof(address);
    socketFD = accept(socketFD, (struct sockaddr *)&address, (socklen_t*)&addrLen);

    if(socketFD < 0) {
        throw std::runtime_error("Failed to accept: " + string(strerror(errno)));
    }

    return socketFD;
}

int NetUtils::socketForSending(Protocol protocol, string address, uint16_t port) {
    int socketFD = createSocket(protocol);

    if(socketFD < 0) {
        throw std::runtime_error("Failed to create sending socket: " + string(strerror(errno)));
    }

    struct sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(address.c_str());

    int connectResult = connect(socketFD, (struct sockaddr *)&server, sizeof(server));

    if(connectResult < 0) {
        throw std::runtime_error("Failed to connect to sending socket: " + string(strerror(errno)));
    }

    return socketFD;
}

void NetUtils::sendMessage(int socket, Token token) {
    std::string toSend = token.toString();
    ssize_t sendResult = write(socket, toSend.c_str(), toSend.size());

    if(sendResult != toSend.size()) {
        throw std::runtime_error("Failed to send message: " + string(strerror(errno)));;
    }
}

int createSocket(Protocol protocol){
    int socketFD = 0;

    if(protocol == TCP){
        socketFD = socket(AF_INET, SOCK_STREAM, 0);
    } else {
        socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    }

    if(socketFD <= 0){
        throw std::runtime_error("Failed to create socket: " + string(strerror(errno)));
    }

    return socketFD;
}

