//
// Created by Yue Xue  on 7/30/25.
//

#include "chunk.h"
#include "vm.h"
#include <memory>
#include "disassembler.h"

int main() {
    VM vm;
    std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>();

    auto index = chunk->add_constant(1);
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
