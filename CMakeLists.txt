cmake_minimum_required(VERSION 3.13)
project(TokenRing)

set(CMAKE_CXX_STANDARD 17)

add_executable(TokenRing main.cpp Token.cpp Token.h utils/InputParser.cpp utils/InputParser.h utils/NetUtils.cpp utils/NetUtils.h utils/StringUtils.cpp utils/StringUtils.h)