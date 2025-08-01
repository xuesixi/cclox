#ifndef CCLOX_COMPILER_H
#define CCLOX_COMPILER_H

#include "chunk.h"
#include "scanner.h"
#include <memory>
#include <string>

class Compiler {
public:
    std::shared_ptr<Chunk> chunk;

    void compile(const std::string &source);

private:

    void error_at_current(const std::string &message);
    void error_at_previous(const std::string &message);
    void error_at(const Token &token, const std::string &message);
    void consume(TokenType type, const std::string &message);

    Token curr;  // 刚刚通过scanner得到的那个token
    Token previous; // curr之前的那一个token
    bool has_error; // 编译的过程中是否出现过错误
    bool panic_mode; // 是否处于panic模式，出现错误时设置true，一路跳过到下一个statement，然后设置为false
};

#endif