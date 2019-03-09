//
// Created by woolfy on 3/9/19.
//

#include "Token.h"

template<class D, class A>
bool Token<D, A>::isTaken() {
    return this->taken;
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
Token<D,A> Token<D, A>::markAsFree() {
    if(this->isTaken()){
        this->isTaken() = false;
        return Token();
    } else {
        throw "Token was free";
    }
}

template<class D, class A>
Token<D, A>::Token(int TTL) {
    this->TTL = TTL;
}

template<class D, class A>
int Token<D, A>::getTTL() {
    return this->TTL;
}


