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

// Lox语言内部的异常
namespace LoxError {
    class TypeError: public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };
}

// 与cclox程序自身相关的异常
namespace InterpreterError {
    class FileOpenFailureError: public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };
}


#endif //CCLOX_ERROR_H
