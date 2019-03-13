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
#include <iostream>
#include <unistd.h>
#include <cstddef>
#include <system_error>
#include <iostream>

#include "InputParser.h"
#include "../Token.h"

class NetUtils {
    public:
        static int socketForReceiving(Protocol protocol, uint16_t port);
        static int socketForSending(Protocol protocol, string address, uint16_t port);
        
        static void sendMessage(int socket, Token token, Input input);
};

#endif //TOKENRING_SENDINGUTILS_H
