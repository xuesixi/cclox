//
// Created by Yue Xue  on 7/30/25.
//

#include "chunk.h"
#include <fmt/core.h>
#include <iostream>
#include <unordered_map>

using std::cout, std::endl;

const std::unordered_map<OpCode, std::string> opcode_names {
        {OpCode::Return, "Return" },
        {OpCode::LoadConstant8, "LoadConstant8"},
        {OpCode::LoadConstant16, "LoadConstant16"}
};

void Chunk::disassemble(const char *name) {
    for (size_t offset = 0; offset < code.size();) {
        offset = disassemble_instruction(offset);
    }
}

size_t Chunk::disassemble_instruction(size_t offset) {
    auto instruction = static_cast<OpCode>(code.at(offset));
    int line = lines.at(offset);
    cout << fmt::format("{:04d}", offset); // byte code offset
    if (offset > 0 && lines.at(offset - 1) != line) {
        cout << "   | "; // if the line num is the same as the previous line, do not repeat
    } else {
        cout << fmt::format("{:04d} ", line); // line num
    }

    switch (instruction) {
        case OpCode::Return:
            return instruction_0_operand(instruction, offset);
        case OpCode::LoadConstant8:
            return instruction_1_operand(instruction, offset);
        case OpCode::LoadConstant16:
            return instruction_2_operand(instruction, offset);
        default:
            cout << fmt::format("unknown instruction: {}\n", offset);
            return offset + 1;
    }
}

size_t Chunk::instruction_0_operand(OpCode instruction, size_t offset) {
    cout << fmt::format("{}\n", opcode_names.at(instruction));
    return offset + 1;
}

size_t Chunk::instruction_1_operand(OpCode instruction, size_t offset) {
    cout << fmt::format("{}: index {}\n", opcode_names.at(instruction), 8);
    return offset + 2;
}

size_t Chunk::instruction_2_operand(OpCode instruction, size_t offset) {
    uint16_t index = u8_to_u16(code.at(offset + 1), code.at(offset + 2));
    cout << fmt::format("{}: index {}\n", opcode_names.at(instruction), index);
    return offset + 3;
}


