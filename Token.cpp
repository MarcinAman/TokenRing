//
// Created by woolfy on 3/9/19.
//

#include "Token.h"

template<class D, class A>
TokenType Token<D, A>::type() {
    return this -> tokentType;
}

template<class D, class A>
D Token<D, A>::setData(D data) {
    this->data = data;
    return this;
}

template<class D, class A>
D Token<D, A>::getData() {
    return this->data;
}

template<class D, class A>
Token<D,A> Token<D, A>::setAddesses(A sourceAddress, A destinationAddress) {
    if(this->isTaken()){
        throw "Token is in use!";
    }

    if(sourceAddress == nullptr || destinationAddress == nullptr){
        throw "Provided address is null!";
    }

    this->sourceAddress = sourceAddress;
    this->destinationAddress = destinationAddress;

    return this;
}

template<class D, class A>
Token<D, A>::Token(int TTL) {
    this->TTL = TTL;
}

template<class D, class A>
int Token<D, A>::getTTL() {
    return this->TTL;
}


