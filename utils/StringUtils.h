//
// Created by woolfy on 3/10/19.
//

#ifndef TOKENRING_STRINGUTILS_H
#define TOKENRING_STRINGUTILS_H

#include <vector>
#include <string>
#include <iostream>


class StringUtils {
public:
    static std::vector<std::string> split(std::string s, const std::string &delimiter);
    static std::string trimEnter(std::string s);
};


#endif //TOKENRING_STRINGUTILS_H
