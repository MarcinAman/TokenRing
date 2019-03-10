//
// Created by woolfy on 3/9/19.
//

#ifndef TOKENRING_TOKEN_H
#define TOKENRING_TOKEN_H
#include <string>
#include <iostream>
#include <cstring>
#include <vector>
#include <stdlib.h>
#include "utils/StringUtils.h"

enum TokenType {
    INIT, MSG, ACK, DISCONNECT
};

class Token {
    private:
        TokenType tokentType;
        std::string data;
        std::string sourceAddress; // a dokladniej sam port
        std::string destinationAddress;
        int TTL;

    public:
        TokenType type();
        void setData(std::string data);
        std::string getData();
        void setTTL(int ttl);
        void setType(TokenType type);
        int getTTL();
        std::string toString();
        void fillFromString(std::string json);
        void setSourceAddress(std::string s);
        void setDestinationAddress(std::string s);

    private:
        std::string typeToString(TokenType token);
        TokenType typeFromString(std::string type);
        std::string getValue(std::string s);

};

#endif //TOKENRING_TOKEN_H
