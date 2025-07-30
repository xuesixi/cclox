//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_ERROR_H
#define CCLOX_ERROR_H

#include <iostream>

inline void compile_error(const std::string &message) {
    std::cout << message << std::endl;
}

#endif //CCLOX_ERROR_H
