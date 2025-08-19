//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_ERROR_H
#define CCLOX_ERROR_H

#include <iostream>
#include <stdexcept>
#include <fmt/core.h>
#include <string>

/**
 * 调用该函数来表示某处出现了预想之外的情况（实现错误）。例如，本不应该到达的语句被运行了。该函数会输出错误信息，然后abort
 */
inline void implementation_error(const std::string &message) {
    std::cerr << fmt::format("there is an implementation: {}", message) << std::endl;
    std::abort();
}

class LoxError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class LoxTypeError : public LoxError {
public:
    using LoxError::LoxError;
};

class LoxNameError : public LoxError {
public:
    using LoxError::LoxError;
};

class LoxArgError : public LoxError {
public:
    using LoxError::LoxError;
};

class LoxStackOverflowError: public LoxError {
public:
    using LoxError::LoxError;
};


class InterpreterError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class FileOpenFailureError : public InterpreterError {
public:
    using InterpreterError::InterpreterError;
};

class CompilerError: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/**
 * 在scan_token中可能会被抛出。只在interpret()中被捕获。
 */
class ScannerError: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ConstantPoolOverflowError : public CompilerError {
public:
    using CompilerError::CompilerError;
};

class DuplicateNameVariableError : public CompilerError {
public:
    using CompilerError::CompilerError;
};

class UsingUninitializedLocalError : public CompilerError {
public:
    using CompilerError::CompilerError;
};

class FmtStringUnbalancedError : public CompilerError {
public:
    using CompilerError::CompilerError;
};

class Uint8OperandOverflowError: public CompilerError {
public:
    using CompilerError::CompilerError;
};

class JumpDistanceOverflowError : public CompilerError {
public:
    using CompilerError::CompilerError;
};

class ConsumePending : std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};


#endif //CCLOX_ERROR_H
