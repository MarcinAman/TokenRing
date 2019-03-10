#include <utility>

#include <iostream>
#include <string>
#include "utils/InputParser.h"
#include "utils/NetUtils.h"

/*
 * 1. Wlacza sie 1 klient, dostaje swoj adress jako wejsciowy.
 * 2. Wlacza sie 2 klient i dostaje port poprzedniego nastepnie wysyla do niego token typu init.
 * 3. Kazdy klient posiada stos z kolejnymi komunikatami jakie ma nadac. Jesli klient otrzyma token init to 
 *  musi wrzucic go na stos i jak dostanie token to zaadresowac do sasiada. 
 * 4. Jesli token wroci (co sprawdzamy przez fakt, ze mamy juz ten adres w kolejce) i dostaniemy token to nadajemy
 *  kolejna wiadomosc ze stosu
 * 5. Po otrzymaniu jakiegokolwiek tokenu zmieniamy jego pole TTL na TTL-- i sprawdzamy czy jest do nas adresowany. 
 *  - Jesli jest to robimy kopie, ustawiamy defaultowy TTL, i zamieniamy adresy oraz typ na ACK, wysylamy
 *  - Jesli nie jest to wysylamy
 * Algorytm dla disconnect dziala tak samo jak dla init
 * 
 * Typy tokenow:
 * INIT, MSG, ACK, DISCONNECT
 *
 * Potrzebujemy sie zabezpieczyc przed sytuacja gdzie stacja koncowa/poczatkowa umarla. Moze jakis TTL?
 *
 *
 * Token służy więc do utworzenia ramki danych. Nadająca stacja zmienia sekwencję SOF, dodaje potrzebne dane, adresuje je
 * i umieszcza z powrotem w sieci. Jeśli stacja nie chce nadawać, może po prostu z powrotem umieścić token w sieci
 * – wtedy otrzyma go kolejna stacja. Gdy ramka dotrze do miejsca przeznaczenia, urządzenie odbierające nie wyciąga ramki z sieci,
 * lecz po prostu kopiuje jej zawartość do bufora w celu dalszego wewnętrznego przetwarzania. W oryginalnej ramce zmieniany jest
 * bit pola sterowania dostępem, co informuje nadawcę, że ramka została odebrana. Potem ramka kontynuuje swoją podróż przez pierścień,
 * dopóki nie powróci do urządzenia, które ją wysłało. Gdy urządzenie ją odbierze, uznaje się, że transmisja zakończyła się sukcesem;
 * zawartość ramki jest kasowana, a sama ramka jest z powrotem przekształcana w token.
 *
 * najpierw jest stan listen. Potem jak dostanie cos na ten socket to tworzy nastepny do wysylania do tego kogos
 */

void sendInitMessage(int sendingSocket, Input input){
    Token token;
    token.setData("penis string is the best");
    token.setDestinationAddress(input.neighbourIpAddess);
    token.setSourceAddress(std::to_string(input.listeningPort));
    token.setType(INIT);
    token.setTTL(10);

    NetUtils::sendMessage(sendingSocket, token);
}

int main(int argc, char *argv[]) {
    Input input = InputParser::parseArguments(argc, argv);

    std::cout << "Initialized with parameters:" << std::endl;
    std::cout << input.toString() << std::endl;

    if(input.listeningPort == input.neighbourPort){
        //this is first, we dont open sending socket because no one is listening
        int receivingSocket = NetUtils::socketForReceiving(input.protocol, static_cast<uint16_t>(input.listeningPort));
        printf("socket recv: %d\n", receivingSocket);

        NetUtils::receiveMessage(receivingSocket, input);
    } else {
        //other nodes
        int sendingSocket = NetUtils::socketForSending(input.protocol, input.neighbourIpAddess, static_cast<uint16_t>(input.neighbourPort));
        printf("sending socket: %d\n", sendingSocket);

        sendInitMessage(sendingSocket, input);

        int receivingSocket = NetUtils::socketForReceiving(input.protocol, static_cast<uint16_t>(input.listeningPort));
        NetUtils::receiveMessage(receivingSocket, input);
    }

    return 0;
}