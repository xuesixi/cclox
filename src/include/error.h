//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_ERROR_H
#define CCLOX_ERROR_H

#include <iostream>
#include <stdexcept>
#include <string>

inline void compile_error(const std::string &message) {
    std::cout << message << std::endl;
}

class LoxTypeError: public std::runtime_error {
public:
    LoxTypeError(const std::string &msg): std::runtime_error(msg) {}
};

#endif //CCLOX_ERROR_H
