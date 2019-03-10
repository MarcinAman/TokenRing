//
// Created by woolfy on 3/9/19.
//

#ifndef TOKENRING_TOKEN_H
#define TOKENRING_TOKEN_H

enum TokenType {
    INIT, MSG, ACK, DISCONNECT
};

template <class D, class A>
class Token {
    private:
        TokenType tokentType;
        D data;
        A sourceAddress;
        A destinationAddress;
        int TTL;

    public:
    explicit Token(int TTL);
        TokenType type();
        D setData(D data);
        D getData();
        Token setAddesses(A sourceAddress, A destinationAddress);
        int getTTL();
};

#endif //TOKENRING_TOKEN_H
