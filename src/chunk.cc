//
// Created by Yue Xue  on 7/30/25.
//

#include "chunk.h"
#include "value.h"
#include <cstddef>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <unordered_map>

using std::cout;

const std::unordered_map<OpCode, std::string> opcode_names {
        {OpCode::Return, "Return" },
        {OpCode::LoadConstant8, "LoadConstant8"},
        {OpCode::LoadConstant16, "LoadConstant16"}
};

static std::string spaces_4("    ");

void Chunk::disassemble(const char *name) {
    cout << fmt::format("the space4 is {}\n", spaces_4);
    cout << fmt::format("--------- start disassembling chunk: {} ---------\n", name);
    for (size_t offset = 0; offset < code.size();) {
        offset = disassemble_instruction(offset);
    }
}

size_t Chunk::disassemble_instruction(size_t offset) {
    auto instruction = static_cast<OpCode>(code.at(offset));
    int line = lines.at(offset);
    cout << fmt::format("{:04d}{}", offset, spaces_4); // byte code offset
    if (offset > 0 && lines.at(offset - 1) == line) {
        cout << fmt::format("   |{}", spaces_4); // if the line num is the same as the previous line, do not repeat it
    } else {
        cout << fmt::format("{:04d}{}", line, spaces_4); // line num
    }

    switch (instruction) {
        case OpCode::Return:
            return instruction_operand_0(instruction, offset);
        case OpCode::LoadConstant8:
            return instruction_operand_1(instruction, offset);
        case OpCode::LoadConstant16:
            return instruction_operand_2(instruction, offset);
        default:
            cout << fmt::format("unknown instruction: {}\n", offset);
            return offset + 1;
    }
}

size_t Chunk::instruction_operand_0(OpCode instruction, size_t offset) {
    cout << fmt::format("{:12}\n", opcode_names.at(instruction));
    return offset + 1;
}

size_t Chunk::instruction_operand_1(OpCode instruction, size_t offset) {
    size_t index = code.at(offset + 1);
    Value value = constants.at(index);
    std::string value_str = value_to_string(value);
    cout << fmt::format("{:12} {}index {}, value {}\n", opcode_names.at(instruction), spaces_4, index, value_str);
    return offset + 2;
}

size_t Chunk::instruction_operand_2(OpCode instruction, size_t offset) {
    uint16_t index = u8_to_u16(code.at(offset + 1), code.at(offset + 2));
    Value value = constants.at(index);
    std::string value_str = value_to_string(value);
    cout << fmt::format("{:12} {}index {}, value {}\n", opcode_names.at(instruction), spaces_4, index, value_str);
    return offset + 3;
}


