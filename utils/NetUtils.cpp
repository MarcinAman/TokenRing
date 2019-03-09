//
// Created by woolfy on 3/9/19.
//

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
        printf("Error code: %s\n", strerror(errno));
        throw "Failed to bind socket to port: \n" + std::to_string(port);
    }

    int listenResult = listen(socketFD, 3); // w sumie to cokolwiek wieksze od 1 bo i tak nie powinnismy miec w kolejce wiecej niz 1

    if(listenResult < 0){
        printf("Error code: %s\n", strerror(errno));
        throw "Failed to listen";
    }

    return socketFD;
}

int NetUtils::socketForSending(Protocol protocol, string address, uint16_t port) {
    int socketFD = createSocket(protocol);

    struct sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(address.c_str());

    int connectResult = connect(socketFD, (struct sockaddr *)&server, sizeof(server));

    if(connectResult < 0) {
        printf("Error code: %s\n", strerror(errno));
        throw "Failed to connect";
    }

    return socketFD;
}

void NetUtils::sendMessage(int socket) {

}

int createSocket(Protocol protocol){
    int socketFD = 0;

    if(protocol == TCP){
        socketFD = socket(AF_INET, SOCK_STREAM, 0);
    } else {
        socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    }

    if(socketFD < 0){
        printf("Error code: %s\n", strerror(errno));
        throw "Failed to create socket";
    }

    return socketFD;
}

