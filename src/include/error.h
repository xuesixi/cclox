//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_ERROR_H
#define CCLOX_ERROR_H

#include <iostream>
#include <stdexcept>
#include <fmt/core.h>
#include <string>

inline void compile_error(const std::string &message) {
    std::cerr << message << std::endl;
}

inline void implementation_error(const std::string &message) {
    std::cerr << fmt::format("there is an implementation: {}", message) << std::endl;
}

class LoxError: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class LoxTypeError : public LoxError {
public:
    using LoxError::LoxError;
};

class InterpreterError: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class FileOpenFailureError: public InterpreterError {
public:
    using InterpreterError::InterpreterError;
};


#endif //CCLOX_ERROR_H
