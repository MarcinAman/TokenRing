//
// Created by woolfy on 3/10/19.
//

#include "StringUtils.h"

std::vector<std::string> StringUtils::split(std::string s, const std::string &delimiter) {
    std::vector<std::string> vec;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        vec.push_back(token);
        s.erase(0, pos + delimiter.length());
    }

    vec.push_back(s);

    return vec;
}

std::string StringUtils::trim(std::string str) {
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last-first+1));
}
