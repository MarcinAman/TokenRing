//
// Created by woolfy on 3/9/19.
//

#ifndef TOKENRING_SENDINGUTILS_H
#define TOKENRING_SENDINGUTILS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>
#include <cstring>
#include <arpa/inet.h>

#include "InputParser.h"

class NetUtils {
    public:
        static int socketForReceiving(Protocol protocol, uint16_t port);
        static int socketForSending(Protocol protocol, string address, uint16_t port);
        static void sendMessage(int socket);
};

#endif //TOKENRING_SENDINGUTILS_H
