#include <utility>

#include <iostream>
#include <string>
#include <csignal>
#include "utils/InputParser.h"
#include "utils/NetUtils.h"
#include <ctime>
#include <sys/epoll.h>
#include <algorithm>


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
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

    cout << "Init message sent" << endl;
}

void closeSockets(int s) {
    if(sendingSocket > 0){
        shutdown(sendingSocket, SHUT_RDWR);
        close(sendingSocket);
    }
    if(receivingSocket > 0){
        shutdown(receivingSocket, SHUT_RDWR);
        close(receivingSocket);
    }

    if(s != -1){
        exit(0);
    }
}

void atExit(){
    closeSockets(0);
}


int main(int argc, char *argv[]) {
    Input input = InputParser::parseArguments(argc, argv);

    std::cout << "Initialized with parameters:" << std::endl;
    std::cout << input.toString() << std::endl;

    sighandler_t t = signal(SIGINT, closeSockets);

    if(t == SIG_ERR){
        throw std::runtime_error("Failed to register sighandler: " + string(strerror(errno)));
    }

    if(atexit(atExit) != 0){
        throw std::runtime_error("Failed to register atexit: " + string(strerror(errno)));
    }

    std::vector<string> knownHosts;


    if(input.listeningPort != input.neighbourPort){
        sendingSocket = NetUtils::socketForSending(input.protocol, input.neighbourIpAddess,
                                                   static_cast<uint16_t>(input.neighbourPort));
        std::cout << "socket send: " + to_string(sendingSocket) << std::endl;

        knownHosts.push_back(input.neighbourIpAddess + ":" + to_string(input.neighbourPort));

        sendInitMessage(sendingSocket, input);
    }

    receivingSocket = NetUtils::socketForReceiving(input.protocol, static_cast<uint16_t>(input.listeningPort));

    std::cout << "socket recv: " + to_string(receivingSocket) << std::endl;

    char dataReceived[1024];

    while(true){
        ssize_t bytesRead = read(receivingSocket, &dataReceived, 1024);

        if(bytesRead == -1){
            throw std::runtime_error("Failed to read from receiving socket: " + string(strerror(errno)));
        }

        if(bytesRead > 0){
            std::string response(dataReceived);
            Token token;
            token.fillFromString(response);
            std::cout << token.toString() << std::endl;

            if(token.type() == INIT && sendingSocket == -1){
                std::string source = token.getSourceAddress();
                knownHosts.push_back(source);

                std::vector<std::string> parsed = StringUtils::split(source,":");

                sendingSocket = NetUtils::socketForSending(input.protocol, parsed.at(0),
                                                           static_cast<uint16_t>(atoi(parsed.at(1).c_str())));
                token.setType(ACK);
            } else if(token.type() == INIT){
                // just send ack, nothing to do
                // only add another host to stack
                knownHosts.push_back(token.getSourceAddress());

            } else if(token.type() == DISCONNECT){
                // check if the node that disconnected is our's destination.
                // If so, update sending socket
                string source = token.getSourceAddress();
                auto f = [&source](string v){
                    return v == source;
                };

                knownHosts.erase(
                        remove_if(knownHosts.begin(), knownHosts.end(), f), knownHosts.end()
                        );

                //after removing send update to others by ignoring this message

            } else if(token.type() == MSG){
                //Is it addressed to me?
                if(token.getDestinationAddress() == "127.0.0.1:"+to_string(input.listeningPort)) {
                    token.setType(ACK);
                    token.setSourceAddress(token.getDestinationAddress());
                    token.setDestinationAddress(token.getSourceAddress());
                    token.setTTL(10);
                }
                // if no just ignore and pass with lower ttl
            } else if(token.type() == ACK){
                // So message got passed.
                // Send an empty one so some1 can send theirs
                token.setTTL(10);
                token.setType(EMPTY);

            } else if(token.type() == EMPTY || token.getTTL() <= 0){
                // empty, rand for sending
                // or ttl expired

                if(rand()%2 == 0){
                    cout << "Client "+ input.id + " decided to send message" << endl;

                    time_t ti;
                    time (&ti);
                    string currentTime = ctime(&ti);

                    token.setData(currentTime);
                    token.setType(MSG);
                    token.setSourceAddress("127.0.0.1:"+to_string(input.listeningPort));

                    unsigned long index = rand() % knownHosts.size();
                    token.setDestinationAddress(knownHosts.at(index));

                    cout << "sending message to: " + knownHosts.at(index) << endl;

                } else{
                    cout << "Client "+ input.id + " decided to pass token" << endl;
                    token.setData("");
                    token.setType(EMPTY);
                }

                token.setTTL(10);
            }

            int ttl = token.getTTL()-1;
            token.setTTL(ttl);

            sleep(1);

            NetUtils::sendMessage(sendingSocket, token);
        }


    }

    return 0;
}

#pragma clang diagnostic pop