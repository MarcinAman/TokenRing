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

int sendingSocket = -1;
int receivingSocket = -1;
int multicastSocket = -1;

string multicastIP = "224.0.0.1";
int multicast_port = 9999;

void sendInitMessage(Input input){
    Token token;
    token.setData("INIT!");
    token.setDestinationAddress(input.neighbourIpAddess+":"+to_string(input.neighbourPort));
    token.setSourceAddress(input.neighbourIpAddess+":"+std::to_string(input.listeningPort));
    token.setType(INIT);
    token.setTTL(10);

    sendingSocket = NetUtils::socketForSending(input.protocol, input.neighbourIpAddess,
                                               static_cast<uint16_t>(input.neighbourPort));

    NetUtils::sendMessage(sendingSocket, token);

    cout << "Init message sent" << endl;

    close(sendingSocket);
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

void initMulti(){
    multicastSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (multicastSocket == -1) {
        throw std::runtime_error("Failed to create socket for multicastIP: " + string(strerror(errno)));
    }
}

void sendMulti(const string &toSend){
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(multicastIP.c_str());
    addr.sin_port = htons(static_cast<uint16_t>(multicast_port));

    if (sendto(multicastSocket,toSend.c_str(), toSend.size(), 0, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Failed to send sthing to multicastIP: " + string(strerror(errno)));
    }
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

    initMulti();

    if(input.listeningPort != input.neighbourPort){

        knownHosts.push_back(input.neighbourIpAddess + ":" + to_string(input.neighbourPort));

        sendInitMessage(input);
        std::cout << "socket send: " + to_string(sendingSocket) << std::endl;
    }

    receivingSocket = NetUtils::socketForReceiving(input.protocol, static_cast<uint16_t>(input.listeningPort));

    std::cout << "socket recv: " + to_string(receivingSocket) << std::endl;

    char dataReceived[1024];
    int tcpInSocket = -1;

    while(true){
        tcpInSocket = accept(receivingSocket, NULL, NULL);

        if(tcpInSocket < 0) {
            throw std::runtime_error("Failed to accept: " + string(strerror(errno)));
        }

        ssize_t bytesRead = read(tcpInSocket, &dataReceived, 1024);

        if(bytesRead == -1){
            throw std::runtime_error("Failed to read from receiving socket: " + string(strerror(errno)));
        }

        if(bytesRead > 0){
            std::string response(dataReceived);
            Token token;
            token.fillFromString(response);
            std::cout << token.toString() << std::endl;

            if(token.type() == INIT &&
            (sendingSocket == -1 ||
            token.getDestinationAddress() == input.neighbourIpAddess+":"+to_string(input.neighbourPort))){
                std::string source = token.getSourceAddress();
                //add to known hosts

                std::vector<std::string> parsed = StringUtils::split(source,":");

                if(atoi(parsed.at(1).c_str()) != input.neighbourPort){
                    knownHosts.push_back(source);
                    input.neighbourPort = atoi(parsed.at(1).c_str());
                    input.neighbourIpAddess = parsed.at(0);
                    cout << "[INIT] changed sending ports to: " +  source << endl;
                } else {
                    cout << "init didnt change porst" << endl;
                }

                sendMulti(input.id);
            } else if(token.type() == EMPTY || token.getTTL() <= 0){
                    // empty, rand for sending
                    // or ttl expired

                    if(rand()%2 == 0){
                        cout << "Client "+ input.id + " decided to send message" << endl;

                        token.setData(to_string(rand()%10000));
                        token.setType(MSG);
                        token.setSourceAddress("127.0.0.1:"+to_string(input.listeningPort));

                        unsigned long index = rand() % knownHosts.size();
                        token.setDestinationAddress(knownHosts.at(index));

                        cout << "sending message to: " + knownHosts.at(index) << endl;
                        sendMulti(input.id);

                    } else{
                        cout << "Client "+ input.id + " decided to pass token" << endl;
                        token.setData("");
                        token.setType(EMPTY);
                    }

                    token.setTTL(10);
                } else if(token.type() == INIT
                && token.getDestinationAddress() == "127.0.0.1:"+to_string(input.listeningPort)){
                //thats mine so i pass it
                if(token.getDestinationAddress() == token.getSourceAddress()){
                    token.setType(EMPTY);
                    token.setTTL(10);
                } else {
                    knownHosts.push_back(token.getSourceAddress());
                    token.setSourceAddress("127.0.0.1:"+to_string(input.listeningPort));
                }
            } else if(token.type() == MSG){
                //Is it addressed to me?
                if(token.getDestinationAddress() == "127.0.0.1:"+to_string(input.listeningPort)) {
                    token.setType(ACK);
                    token.setSourceAddress(token.getDestinationAddress());
                    token.setDestinationAddress(token.getSourceAddress());
                    token.setTTL(10);
                    sendMulti(input.id);
                }
                // if no just ignore and pass with lower ttl
            } else if(token.type() == ACK){
                // So message got passed.
                // Send an empty one so some1 can send theirs
                token.setTTL(10);
                token.setType(EMPTY);

            }

            int ttl = token.getTTL()-1;
            token.setTTL(ttl);

            sleep(1);

            sendingSocket = NetUtils::socketForSending(input.protocol, input.neighbourIpAddess,
                                                       static_cast<uint16_t>(input.neighbourPort));

            NetUtils::sendMessage(sendingSocket, token);

            close(sendingSocket);
        }
    }

    return 0;
}

#pragma clang diagnostic pop