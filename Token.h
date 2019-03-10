//
// Created by woolfy on 3/9/19.
//

#ifndef TOKENRING_TOKEN_H
#define TOKENRING_TOKEN_H
#include <string>
#include <iostream>
#include <cstring>

enum TokenType {
    INIT, MSG, ACK, DISCONNECT
};

class Token {
    private:
        TokenType tokentType;
        std::string data;
        std::string sourceAddress;
        std::string destinationAddress;
        int TTL;

    public:
        TokenType type();
        void setData(std::string data);
        std::string getData();
        void setTTL(int ttl);
        void setType(TokenType type);
        void setAddesses(std::string sourceAddress, std::string destinationAddress);
        int getTTL();
        std::string toString();
        void fillFromString(std::string json);

    private:
        std::string typeToString(TokenType token);
};

#endif //TOKENRING_TOKEN_H
