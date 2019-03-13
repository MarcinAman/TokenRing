#include <utility>

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
        case EMPTY: return "EMPTY";
    }
}

void Token::setTTL(int ttl) {
    this->TTL = ttl;
}

void Token::setType(TokenType type) {
    this->tokentType = type;
}

void Token::fillFromString(std::string s) {
    std::string delimiter = "|";

    std::vector<std::string> splitted = StringUtils::split(std::move(s), delimiter);

    for(const std::string &token : splitted){
        if(token.rfind("data") == 0){ //starts with
            this->setData(getValue(token));
        } else if(token.rfind("TTL") == 0){
            int ttl = atoi(getValue(token).c_str());
            this->setTTL(ttl);
        } else if(token.rfind("sourceAddress") == 0){
            this->setSourceAddress(getValue(token));
        } else if(token.rfind("destinationAddress") == 0){
            this->setDestinationAddress(getValue(token));
        } else if(token.rfind("type") == 0){
            this->setType(typeFromString(getValue(token)));
        } else {
            throw std::runtime_error("Token not found: " + token);
        }
    }
}

TokenType Token::typeFromString(std::string type) {
    if(type.rfind("INIT") == 0){
        return INIT;
    }
    if(type.rfind("MSG") == 0){
        return MSG;
    }
    if(type.rfind("ACK") == 0){
        return ACK;
    }
    if(type.rfind("ACK") == 0){
        return ACK;
    }
    if(type.rfind("EMPTY") == 0){
        return EMPTY;
    }

    return MSG;
}

std::string Token::getValue(std::string s) {
    std::vector<std::string> data = StringUtils::split(std::move(s), "=");

    return data.at(1);
}

void Token::setSourceAddress(std::string s) {
    this->sourceAddress = std::move(s);
}

void Token::setDestinationAddress(std::string s) {
    this->destinationAddress = std::move(s);
}

std::string Token::getSourceAddress() {
    return this->sourceAddress;
}

std::string Token::getDestinationAddress() {
    return this->destinationAddress;
}
