//
// Created by Yue Xue  on 7/30/25.
//

#include "chunk.h"
#include "vm.h"
#include <memory>
#include "disassembler.h"

void repl() {
    std::string buffer;
    while (true) {
        std::getline(std::cin, buffer);
    }
}

void run_file(const std::string &path) {
    std::string content = std::move(read_file(path));
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

int main(int argc, const char **args) {
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        run_file(args[1]);
    } else {
        std::cerr << "error: expect zero or more arguments\n";
    }
}
