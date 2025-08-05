#include "disassembler.h"
#include "chunk.h"
#include <fmt/core.h>
#include <unordered_map>

using std::cout;

/**
 * 指令和其字符串表达的映射
 */
const std::unordered_map<OpCode, std::string> opcode_names{
        {OpCode::Return,        "Return"},
        {OpCode::LoadConstant,  "LoadConstant"},
        {OpCode::LoadConstant2, "LoadConstant2"},
        {OpCode::LoadImmediate, "LoadImmediate"},
        {OpCode::Negate,        "Negate"},
        {OpCode::Add,           "Add"},
        {OpCode::Subtract,      "Subtract"},
        {OpCode::Multipy,       "Multipy"},
        {OpCode::Divide,        "Divide"},
        {OpCode::LoadNil,       "LoadNil"},
        {OpCode::LoadTrue,      "LoadTrue"},
        {OpCode::LoadFalse,     "LoadFalse"},
        {OpCode::Not,           "Not"},
        {OpCode::Less,          "Less"},
        {OpCode::Greater,       "Greater"},
        {OpCode::Equal,         "Equal"},
        {OpCode::Print,         "Print"},
        {OpCode::Pop,           "Pop"},
        {OpCode::DefineGlobal,  "DefineGlobal"},
        {OpCode::LoadGlobal, "LoadGlobal"},
};

// 四个空格。格式化的时候偶尔会用到。
static std::string spaces_4("    ");

void Disassembler::disassemble(const char *name) {
    cout << fmt::format("--------- start disassembling chunk: {} ---------\n", name);

    // offset代表当前指令在chunk的字节码中的索引。有的指令有额外的参数（占用超过一个字节），
    // 因此下一个指令的索引将由disassemble_instruction函数判断
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
        case OpCode::LoadConstant:
            return instruction_constant_operand_1(instruction, offset);
        case OpCode::LoadConstant2:
            return instruction_constant_operand_2(instruction, offset);
        case OpCode::LoadImmediate:
            return instruction_load_immediate(instruction, offset);
        case OpCode::DefineGlobal:
        case OpCode::LoadGlobal:
            return instruction_identifier_operand_2(instruction, offset);
        case OpCode::Return:
        case OpCode::Negate:
        case OpCode::Add:
        case OpCode::Subtract:
        case OpCode::Multipy:
        case OpCode::Divide:
        case OpCode::LoadNil:
        case OpCode::LoadTrue:
        case OpCode::LoadFalse:
        case OpCode::Less:
        case OpCode::Greater:
        case OpCode::Equal:
        case OpCode::Not:
        case OpCode::Print:
        case OpCode::Pop:
            return instruction_operand_0(instruction, offset);
//        default:
//            cout << fmt::format("unknown instruction: {}\n", offset);
//            return offset + 1;
    }
}

size_t Disassembler::instruction_operand_0(OpCode instruction, size_t offset) {
    cout << fmt::format("{:12}\n", opcode_names.at(instruction));
    return offset + 1;
}

size_t Disassembler::instruction_constant_operand_1(OpCode instruction, size_t offset) {
    size_t index = chunk->code.at(offset + 1);
    Value value = chunk->constants.at(index);
    std::string value_str = LoxValue::to_string(value);
    cout << fmt::format("{:12} {}index: {}, value: {}\n", opcode_names.at(instruction), spaces_4, index, value_str);
    return offset + 2;
}

size_t Disassembler::instruction_load_immediate(OpCode instruction, size_t offset) {
    size_t index = chunk->code.at(offset + 1);
    Value value = Chunk::read_immediate(index);
    std::string value_str = LoxValue::to_string(value);
    cout << fmt::format("{:12} {} value: {}\n", opcode_names.at(instruction), spaces_4, value_str);
    return offset + 2;
}

size_t Disassembler::instruction_constant_operand_2(OpCode instruction, size_t offset) {
    uint16_t index = u8_to_u16(chunk->code.at(offset + 1), chunk->code.at(offset + 2));
    Value value = chunk->constants.at(index);
    std::string value_str = LoxValue::to_string(value);
    cout << fmt::format("{:12} {}index: {}, value: {}\n", opcode_names.at(instruction), spaces_4, index, value_str);
    return offset + 3;
}

size_t Disassembler::instruction_identifier_operand_2(OpCode instruction, size_t offset) {
    uint16_t key = u8_to_u16(chunk->code.at(offset + 1), chunk->code.at(offset + 2));
    std::string identifier = chunk->read_identifier(key);
    cout << fmt::format("{:12} {}identifier: {}\n", opcode_names.at(instruction), spaces_4, identifier);
    return offset + 3;
}

