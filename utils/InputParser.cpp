#include <utility>

#include <utility>

//
// Created by Marcin Aman on 2019-03-09.
//

#include "InputParser.h"
#include <vector>
#include <iostream>

Input InputParser::parseArguments(int argc, char *argv[]) {

    map<string, string> commandLine;

    for(int i = 1; i<argc ; i += 2){
        string key = argv[i];
        string value = argv[i+1];

        commandLine[key] = value;
    }

    Input parsedCommandLine;

    parsedCommandLine.doesHaveToken = commandLine["-token"] == "T";
    parsedCommandLine.id = commandLine["-id"];
    parsedCommandLine.listeningPort = std::stoi(commandLine["-port"]);
    parsedCommandLine.neighbourIpAddess = commandLine["-address"];

    if(commandLine["-protocol"] == "TCP"){
        parsedCommandLine.protocol = TCP;
    } else {
        parsedCommandLine.protocol = UDP;
    }

    return parsedCommandLine;
}

Input::Input(std::string id, int listeningPort, std::string neighbourIpAddess, bool doesHaveToken, Protocol protocol) {
    this->id = std::move(id);
    this->listeningPort = listeningPort;
    this->doesHaveToken = doesHaveToken;
    this->protocol = protocol;
    this->neighbourIpAddess = std::move(neighbourIpAddess);
}

Input::Input() {
    this -> id = "";
    this->doesHaveToken = false;
    this->listeningPort = 0;
    this->neighbourIpAddess = "";
}

std::string Input::toString() {
    return "Id: " + this->id + ", Token: " + (this->doesHaveToken ? "true":"false") + ", listeningPort: "
    + std::to_string(this->listeningPort) + ", neighbourIpAddess: " + this->neighbourIpAddess + ", protocol: " + (this->protocol == TCP ? "TCP" : "UDP");
}
