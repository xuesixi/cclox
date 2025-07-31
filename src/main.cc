//
// Created by Yue Xue  on 7/30/25.
//

#include "cclox_util.h"
#include "chunk.h"
#include <fmt/core.h>
#include "vm.h"
#include <error.h>
#include <memory>
#include "disassembler.h"
#include "scanner.h"

void repl() {
    std::string buffer;
    while (true) {
        std::getline(std::cin, buffer);
    }
}

void run_file(const std::string &path) {
    try {
        std::string content = read_file(path);
    } catch (InterpreterError::FileOpenFailureError &err) {
        std::cerr << err.what() << std::endl;
    }
}

void testVM() {
    VM vm;
    std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>();

    size_t index = chunk->add_constant(1);
    chunk->write_opcode(OpCode::LoadConstant8, 123);
    chunk->write_index(index, 123);

    index = chunk->add_constant(3);
    chunk->write_opcode(OpCode::LoadConstant8, 123);
    chunk->write_index(index, 123);

    chunk->write_opcode(OpCode::Add, 123);

    index = chunk->add_constant(4);
    chunk->write_opcode(OpCode::LoadConstant8, 124);
    chunk->write_index(index, 124);

    index = chunk->add_constant(5);
    chunk->write_opcode(OpCode::LoadConstant8, 124);
    chunk->write_index(index, 124);

    chunk->write_opcode(OpCode::Multipy, 124);
    chunk->write_opcode(OpCode::Subtract, 124);

    chunk->write_opcode(OpCode::Return, 125);

    // Disassembler disass(chunk);
    // disass.disassemble("test chunk");

    vm.interpret(chunk);
}

void test_scanner(const std::string &path) {
    Scanner scanner(read_file(path));
    while (scanner.has_more()) {
        Token token = scanner.scan_token();
        std::cout << token.to_string() << std::endl;
    }
}

void go(int argc, const char **args) {
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        run_file(args[1]);
    } else {
        std::cerr << "error: expect zero or more arguments\n";
    }
}

int main(int argc, const char **args) {
    test_scanner("/Users/yuexue/Codes/try/cclox/build/hello.lox");
}
