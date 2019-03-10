//
// Created by Marcin Aman on 2019-03-09.
//

#ifndef TOKENRING_INPUTPARSER_H
#define TOKENRING_INPUTPARSER_H
#include <string>
#include <map>

enum Protocol {
    TCP, UDP
};

class Input {
    public:
        std::string id;
        int listeningPort;
        std::string neighbourIpAddess;
        int neighbourPort;
        bool doesHaveToken;
        Protocol protocol;

        Input();

        std::string toString();
};

using namespace std;


class InputParser {
    public:
        static Input parseArguments(int argc, char *argv[]);
};


#endif //TOKENRING_INPUTPARSER_H
