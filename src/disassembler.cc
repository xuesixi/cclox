#include "disassembler.h"
#include "chunk.h"
#include <fmt/core.h>
#include <unordered_map>

using std::cout;

const std::unordered_map<OpCode, std::string> opcode_names {
        {OpCode::Return, "Return" },
        {OpCode::LoadConstant8, "LoadConstant8"},
        {OpCode::LoadConstant16, "LoadConstant16"},
        {OpCode::Negate, "Negate"},
        {OpCode::Add, "Add"},
        {OpCode::Subtract, "Subtract"},
        {OpCode::Multipy, "Multipy"},
        {OpCode::Divide, "Divide"},
};

static std::string spaces_4("    ");

void Disassembler::disassemble(const char *name) {
    cout << fmt::format("--------- start disassembling chunk: {} ---------\n", name);
    for (size_t offset = 0; offset < chunk->code.size();) {
        offset = disassemble_instruction(offset);
    }
    cout << fmt::format("---------- end disassembling chunk: {} ----------\n", name);
}

size_t Disassembler::disassemble_instruction(size_t offset) {
    auto instruction = static_cast<OpCode>(chunk->code.at(offset));
    int line = chunk->lines.at(offset);
    cout << fmt::format("{:04d}{}", offset, spaces_4); // byte code offset
    if (offset > 0 && chunk->lines.at(offset - 1) == line) {
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
        case OpCode::Negate:
            return instruction_operand_0(instruction, offset);
        case OpCode::Add:
            return instruction_operand_0(instruction, offset);
        case OpCode::Subtract:
            return instruction_operand_0(instruction, offset);
        case OpCode::Multipy:
            return instruction_operand_0(instruction, offset);
        case OpCode::Divide:
            return instruction_operand_0(instruction, offset);
        default:
            cout << fmt::format("unknown instruction: {}\n", offset);
            return offset + 1;
    }
}

size_t Disassembler::instruction_operand_0(OpCode instruction, size_t offset) {
    cout << fmt::format("{:12}\n", opcode_names.at(instruction));
    return offset + 1;
}

size_t Disassembler::instruction_operand_1(OpCode instruction, size_t offset) {
    size_t index = chunk->code.at(offset + 1);
    Value value = chunk->constants.at(index);
    std::string value_str = LoxValue::to_string(value);
    cout << fmt::format("{:12} {}index {}, value {}\n", opcode_names.at(instruction), spaces_4, index, value_str);
    return offset + 2;
}

size_t Disassembler::instruction_operand_2(OpCode instruction, size_t offset) {
    uint16_t index = u8_to_u16(chunk->code.at(offset + 1), chunk->code.at(offset + 2));
    Value value = chunk->constants.at(index);
    std::string value_str = LoxValue::to_string(value);
    cout << fmt::format("{:12} {}index {}, value {}\n", opcode_names.at(instruction), spaces_4, index, value_str);
    return offset + 3;
}

