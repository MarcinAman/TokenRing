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

typedef struct Input {
    std::string id;
    int listeningPort;
    std::string neighbourIpAddess;
    bool doesHaveToken;
    Protocol protocol;
} Input;

using namespace std;


class InputParser {
    public:
        static Input parseArguments(int argc, char *argv[]);
};


#endif //TOKENRING_INPUTPARSER_H