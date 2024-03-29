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

void sendInitMessage(Input input){
    Token token;
    token.setData("INIT!");
    token.setDestinationAddress(input.neighbourIpAddess+":"+to_string(input.neighbourPort));
    token.setSourceAddress(input.neighbourIpAddess+":"+std::to_string(input.listeningPort));
    token.setType(INIT);
    token.setTTL(10);

    sendingSocket = NetUtils::socketForSending(input.protocol, input.neighbourIpAddess,
                                               static_cast<uint16_t>(input.neighbourPort));

    NetUtils::sendMessage(sendingSocket, token, input);

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

string getCurrentAddress(Input input){
    return "127.0.0.1:"+to_string(input.listeningPort);
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
    addr.sin_addr.s_addr = inet_addr("224.0.0.1");
    addr.sin_port = htons(9099);

    ssize_t multi = sendto(multicastSocket, toSend.c_str(), 1024,
                           0, (const struct sockaddr *) &addr,
                           sizeof(addr));

    if (multi <= 0) {
        throw std::runtime_error("Failed to send sthing to multicastIP: " + string(strerror(errno)));
    }



    cout << "log sent: " + to_string(multi) << endl;
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
        for (char &i : dataReceived) {
            i = 0;
        }

        ssize_t bytesRead = 0;

        if(input.protocol == TCP){
            tcpInSocket = accept(receivingSocket, NULL, NULL);

            if(tcpInSocket < 0) {
                throw std::runtime_error("Failed to accept: " + string(strerror(errno)));
            }

            bytesRead = read(tcpInSocket, &dataReceived, 1024);

            if(bytesRead == -1){
                throw std::runtime_error("Failed to read from receiving socket: " + string(strerror(errno)));
            }
        } else {
            struct sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr(input.neighbourIpAddess.c_str());
            addr.sin_port = htons(static_cast<uint16_t>(input.neighbourPort));

            socklen_t s = sizeof(addr);

            bytesRead = recvfrom(receivingSocket, &dataReceived, 1024,
                     MSG_WAITALL, ( struct sockaddr *) &addr,
                     &s);
        }


        if(bytesRead > 0){
            std::string response(dataReceived);
            Token token;
            token.fillFromString(response);
            std::cout << token.toString() << std::endl;

            sendMulti(input.toString());

            if(token.getTTL() <= 0){
                token.setType(EMPTY);
            }

            if(token.type() == INIT){
                // there is a client that would like to send to my neighbour:
                if(token.getDestinationAddress() == input.neighbourIpAddess+":"+to_string(input.neighbourPort)
                && token.getDestinationAddress() != getCurrentAddress(input)
                && token.getData() != getCurrentAddress(input)){
                    //update sending socket:

                    std::string source = token.getData();

                    std::vector<std::string> parsed = StringUtils::split(source,":");

                    if(parsed.size() == 2){
                        knownHosts.push_back(source);
                        input.neighbourPort = atoi(parsed.at(1).c_str());
                        input.neighbourIpAddess = parsed.at(0);

                        cout << "[INIT1] changed sending ports to: " +  source << endl;
                    } else {
                        token.setTTL(10);
                        token.setType(EMPTY);
                    }

                } else if(token.getDestinationAddress() == getCurrentAddress(input)){

                    if(knownHosts.empty()){
                        std::string source = token.getSourceAddress();

                        std::vector<std::string> parsed = StringUtils::split(source,":");

                        knownHosts.push_back(source);
                        input.neighbourPort = atoi(parsed.at(1).c_str());
                        input.neighbourIpAddess = parsed.at(0);

                        cout << "[INIT2] changed sending ports to: " +  source << endl;
                        token.setTTL(10);
                        token.setType(EMPTY);

                        token.setSourceAddress(getCurrentAddress(input));
                    } else {
                        //it is for me and i send it all around
                        if(token.getDestinationAddress() == token.getSourceAddress()){
                            // it was sent all around
                            token.setType(EMPTY);
                            token.setTTL(10);
                            cout << "token returned from all around" << endl;
                        } else {
                            token.setData(token.getSourceAddress());
                            knownHosts.push_back(token.getSourceAddress());
                            token.setSourceAddress(token.getDestinationAddress());
                            cout << "token send all around" << endl;
                            token.setTTL(10);
                        }
                    }
                }
            } else if(token.type() == MSG) {
                cout << "Got message: " + token.getData() << endl;

                token.setType(ACK);
                token.setTTL(10);
                string s = token.getDestinationAddress();
                token.setDestinationAddress(token.getSourceAddress());
                token.setSourceAddress(s);
            } else if(token.type() ==  ACK) {
                token.setType(EMPTY);
                token.setTTL(10);
                token.setSourceAddress(getCurrentAddress(input));
            } else if(token.type() == EMPTY){
                    if(rand()%2 == 0){
                        cout << "Client "+ input.id + " decided to send message" << endl;

                        token.setData(to_string(rand()%10000));
                        token.setType(MSG);
                        token.setSourceAddress("127.0.0.1:"+to_string(input.listeningPort));
                        unsigned long index = rand() % knownHosts.size();
                        token.setDestinationAddress(knownHosts.at(index));

                    } else {
                        cout << "Client "+ input.id + " decided to pass token" << endl;
                        token.setData("");
                        token.setType(EMPTY);
                    }

                    token.setTTL(10);

                } else {
                cout << "Passed: " + token.toString() << endl;
            }


            int ttl = token.getTTL()-1;
            token.setTTL(ttl);

            sleep(1);

            sendingSocket = NetUtils::socketForSending(input.protocol, input.neighbourIpAddess,
                                                       static_cast<uint16_t>(input.neighbourPort));

            NetUtils::sendMessage(sendingSocket, token, input);

            close(sendingSocket);
        }
    }

    return 0;
}

#pragma clang diagnostic pop