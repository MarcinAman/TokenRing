#include <utility>

//
// Created by woolfy on 3/9/19.
//

#include "Token.h"

TokenType Token::type() {
    return this -> tokentType;
}

void Token::setData(std::string data) {
    this->data = std::move(data);
}

std::string Token::getData() {
    return this->data;
}

void Token::setAddesses(std::string sourceAddress, std::string destinationAddress) {
    if(sourceAddress.empty() || destinationAddress.empty()){
        throw "Provided address is empty!";
    }

    this->sourceAddress = sourceAddress;
    this->destinationAddress = destinationAddress;
}

int Token::getTTL() {
    return this->TTL;
}

std::string Token::toString() {
    return "data="+this->data+"|TTL="+std::to_string(this->getTTL())+"|sourceAddress="+this->sourceAddress+"|destinationAddress="
    +this->destinationAddress+"|type="+typeToString(this->type());
}

std::string Token::typeToString(TokenType token) {
    switch(token){
        case INIT: return "INIT";
        case MSG: return "MSG";
        case ACK: return "ACK";
        case DISCONNECT: return "DISCONNECT";
    }
}

void Token::setTTL(int ttl) {
    this->TTL = ttl;
}

void Token::setType(TokenType type) {
    this->tokentType = type;
}

void Token::fillFromString(std::string json) {

}


