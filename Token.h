//
// Created by woolfy on 3/9/19.
//

#ifndef TOKENRING_TOKEN_H
#define TOKENRING_TOKEN_H

template <class D, class A>
class Token {
    private:
        bool taken;
        D data;
        A sourceAddress;
        A destinationAddress;
        int TTL;

    public:
    explicit Token(int TTL);
        bool isTaken();
        D setData(D data);
        D getData();
        Token setAddesses(A sourceAddress, A destinationAddress);
        Token markAsFree();
        int getTTL();
};

#endif //TOKENRING_TOKEN_H
