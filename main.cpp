#include <utility>

#include <iostream>
#include <string>
#include <csignal>
#include "utils/InputParser.h"
#include "utils/NetUtils.h"
#include <ctime>

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

int sendingSocket = -1;
int receivingSocket = -1;

void sendInitMessage(int sendingSocket, Input input){
    Token token;
    token.setData("penis string is the best");
    token.setDestinationAddress(input.neighbourIpAddess+":"+to_string(input.neighbourPort));
    token.setSourceAddress(input.neighbourIpAddess+":"+std::to_string(input.listeningPort));
    token.setType(INIT);
    token.setTTL(10);

    NetUtils::sendMessage(sendingSocket, token);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void receiveMessage(Input input) {
    char dataReceived[1024];


    while(true){
        ssize_t bytesRead = read(receivingSocket, &dataReceived, 1024);

        if(bytesRead > 0){
            std::string response(dataReceived);
            Token token;
            token.fillFromString(response);
            std::cout << token.toString() << std::endl;

            if(token.type() == INIT && sendingSocket == -1){
                std::string source = token.getSourceAddress();

                std::vector<std::string> parsed = StringUtils::split(source,":");

                sendingSocket = NetUtils::socketForSending(input.protocol, parsed.at(0),
                        static_cast<uint16_t>(atoi(token.getSourceAddress().c_str())));
                token.setType(ACK);

                sleep(1);

                NetUtils::sendMessage(sendingSocket, token);
            } else if(token.type() == INIT) {

            } else if(token.type() == DISCONNECT){

            } else if(token.type() == MSG){
                if(token.getDestinationAddress() == "127.0.0.1:"+to_string(input.listeningPort)){
                    token.setType(ACK);
                    token.setSourceAddress(token.getDestinationAddress());
                    token.setDestinationAddress(token.getSourceAddress());
                } else {
                    if(token.getTTL() == 0){
                        token.setTTL(10);
                        token.setType(MSG);
                        token.setSourceAddress("127.0.0.1:"+to_string(input.listeningPort));
                        token.setDestinationAddress(input.neighbourIpAddess);

                        time_t wskaznik;
                        time (&wskaznik);
                        string currentTime= ctime (&wskaznik);

                        token.setData(currentTime);
                    }
                  token.setTTL(token.getTTL()-1);
                }

                sleep(1);

                NetUtils::sendMessage(sendingSocket, token);
            } else {
                //ACK

                token.setType(MSG);
                token.setSourceAddress(token.getDestinationAddress());
                token.setDestinationAddress(token.getSourceAddress());

                time_t t;
                time (&t);
                string currentTime= ctime (&t);

                token.setData(currentTime);
                token.setTTL(10);

                sleep(1);
                NetUtils::sendMessage(sendingSocket, token);
            }

        }
    }

}
#pragma clang diagnostic pop

void closeSockets(int sig){
    shutdown(sendingSocket, SHUT_RDWR);
    close(sendingSocket);

    shutdown(receivingSocket, SHUT_RDWR);
    close(receivingSocket);

    exit(0);
}

int main(int argc, char *argv[]) {
    Input input = InputParser::parseArguments(argc, argv);

    std::cout << "Initialized with parameters:" << std::endl;
    std::cout << input.toString() << std::endl;

    signal(SIGINT, closeSockets);

    if(input.listeningPort == input.neighbourPort){
        //this is first, we dont open sending socket because no one is listening
        receivingSocket = NetUtils::socketForReceiving(input.protocol, static_cast<uint16_t>(input.listeningPort));
        printf("socket recv: %d\n", receivingSocket);

        receiveMessage(input);
    } else {
        //other nodes

        sendingSocket = NetUtils::socketForSending(input.protocol, input.neighbourIpAddess, static_cast<uint16_t>(input.neighbourPort));
        printf("sending socket: %d\n", sendingSocket);

        sendInitMessage(sendingSocket, input);

        receivingSocket = NetUtils::socketForReceiving(input.protocol, static_cast<uint16_t>(input.listeningPort));
        receiveMessage(input);
    }

    return 0;
}