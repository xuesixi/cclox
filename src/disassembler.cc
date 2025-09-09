#include "disassembler.h"
#include "chunk.h"
#include <fmt/core.h>
#include <unordered_map>

#include "compiler.h"
#include "stringintern.h"

/**
 * 指令和其字符串表达的映射
 */
const std::unordered_map<Opcode, std::string> opcode_names{
    {Opcode::Return, "Return"},
    {Opcode::LoadConstant, "LoadConstant"},
    {Opcode::LoadConstant2, "LoadConstant2"},
    {Opcode::LoadImmediate, "LoadImmediate"},
    {Opcode::Negate, "Negate"},
    {Opcode::Add, "Add"},
    {Opcode::Subtract, "Subtract"},
    {Opcode::Multipy, "Multipy"},
    {Opcode::Divide, "Divide"},
    {Opcode::Power, "Power"},
    {Opcode::LoadNil, "LoadNil"},
    {Opcode::LoadTrue, "LoadTrue"},
    {Opcode::LoadEmptyString, "LoadEmptyString"},
    {Opcode::LoadFalse, "LoadFalse"},
    {Opcode::Not, "Not"},
    {Opcode::Less, "Less"},
    {Opcode::Greater, "Greater"},
    {Opcode::Equal, "Equal"},
    {Opcode::Print, "Print"},
    {Opcode::Pop, "Pop"},
    {Opcode::DefineGlobal, "DefineGlobal"},
    {Opcode::LoadGlobal, "LoadGlobal"},
    {Opcode::SetGlobal, "SetGlobal"},
    {Opcode::LoadLocal, "LoadLocal"},
    {Opcode::SetLocal, "SetLocal"},
    {Opcode::LoadCaptured, "LoadCaptured"},
    {Opcode::SetCaptured, "SetCaptured"},
    {Opcode::ClearN, "ClearN"},
    {Opcode::Jump, "Jump"},
    {Opcode::JumpIfPopFalse, "JumpIfPopFalse"},
    {Opcode::JumpBack, "JumpBack"},
    {Opcode::JumpIfFalse, "JumpIfFalse"},
    {Opcode::JumpIfTrue, "JumpIfTrue"},
    {Opcode::Call, "Call"},
    {Opcode::MakeClosure, "MakeClosure"},
    {Opcode::Recur, "Recur"},
    {Opcode::StringConcat, "StringConcat"},
    {Opcode::MakeClass, "MakeClass"},
    {Opcode::LoadField, "LoadField"},
    {Opcode::SetField, "SetField"},
    {Opcode::MethodBind, "MethodBind"},
    {Opcode::MethodInvoke, "MethodInvoke"},
    {Opcode::As, "As"},
    {Opcode::Is, "Is"},
};

// 四个空格。格式化的时候会用到。
static std::string spaces_4("    ");

void Disassembler::disassemble(const std::string &name) {
    *out << fmt::format("--------- start disassembling chunk: {} ---------\n", name);

    // offset代表当前指令在chunk的字节码中的索引。有的指令有额外的参数（占用超过一个字节），
    // 因此下一个指令的索引将由disassemble_instruction函数判断
    for (size_t offset = 0; offset < chunk->code.size();) {
        offset = disassemble_instruction(offset);
    }
    *out << fmt::format("---------- end disassembling chunk: {} ----------\n\n", name);
}

size_t Disassembler::disassemble_instruction(size_t offset) {
    const auto instruction = static_cast<Opcode>(chunk->code.at(offset));
    int line = chunk->lines.at(offset);

    // 字节码偏移值
    *out << fmt::format("{:04d}{}", offset, spaces_4);

    if (line == -1 || (offset > 0 && chunk->lines.at(offset - 1) == line) ) {
        // 如果行号为-1，或者行号和之前行相同，则不重复打印
        *out << fmt::format("   |{}", spaces_4);
    } else {
        *out << fmt::format("{:04d}{}", line, spaces_4); // 行号
    }

    switch (instruction) {
        case Opcode::LoadConstant:
            return constant_index_1(instruction, offset);
        case Opcode::LoadConstant2:
            return constant_index_2(instruction, offset);
        case Opcode::As:
        case Opcode::Is:
            return constant_index_2_as_type(instruction, offset);
        case Opcode::LoadImmediate:
            return load_immediate(instruction, offset);
        case Opcode::DefineGlobal:
        case Opcode::LoadGlobal:
        case Opcode::SetGlobal:
            return instruction_identifier_operand_2(instruction, offset);
        case Opcode::Return:
        case Opcode::Negate:
        case Opcode::Add:
        case Opcode::Subtract:
        case Opcode::Multipy:
        case Opcode::Divide:
        case Opcode::Power:
        case Opcode::LoadNil:
        case Opcode::LoadTrue:
        case Opcode::LoadEmptyString:
        case Opcode::LoadFalse:
        case Opcode::Less:
        case Opcode::Greater:
        case Opcode::Equal:
        case Opcode::Not:
        case Opcode::Print:
        case Opcode::Pop:
            return no_operand(instruction, offset);
        case Opcode::ClearN:
        case Opcode::Call:
        case Opcode::SetCaptured:
        case Opcode::LoadCaptured:
        case Opcode::Recur:
        case Opcode::StringConcat:
        case Opcode::LoadLocal:
        case Opcode::SetLocal:
        case Opcode::LoadField:
        case Opcode::SetField:
            return single_operand(instruction, offset);
        case Opcode::JumpIfPopFalse:
        case Opcode::JumpIfFalse:
        case Opcode::JumpIfTrue:
        case Opcode::Jump:
            return jump_forward(instruction, offset);
        case Opcode::JumpBack:
            return jump_back(instruction, offset);
        case Opcode::MakeClosure:
            return make_closure(instruction, offset);
        case Opcode::MakeClass:
            return make_class(instruction, offset);
        case Opcode::MethodBind:
            return method_bind(instruction, offset);
        case Opcode::MethodInvoke:
            return method_invoke(instruction, offset);
        default:
            ASSERT_UNREACHABLE();
    }
}

size_t Disassembler::no_operand(Opcode instruction, size_t offset) const {
    *out << fmt::format("{:18}\n", opcode_names.at(instruction));
    return offset + 1;
}

size_t Disassembler::constant_index_1(Opcode instruction, size_t offset) const {
    size_t index = chunk->code.at(offset + 1);
    const Value value = chunk->constants.at(index);
    std::string value_str = LoxValue::to_visual_string(value);
    *out << fmt::format("{:18} {} index: {}, value: {}\n", opcode_names.at(instruction), spaces_4, index, value_str);
    return offset + 2;
}

size_t Disassembler::single_operand(Opcode instruction, size_t offset) const {
    uint8_t value = chunk->code.at(offset + 1);
    *out << fmt::format("{:18} {} {}\n", opcode_names.at(instruction), spaces_4, value);
    return offset + 2;
}

size_t Disassembler::jump_forward(Opcode instruction, size_t offset) const {
    const uint16_t distance = u8_to_u16(chunk->code.at(offset + 1), chunk->code.at(offset + 2));
    size_t destination = offset + 3 + distance;
    *out << fmt::format("{:18} {} -> {}\n", opcode_names.at(instruction), spaces_4, destination);
    return offset + 3;
}

size_t Disassembler::jump_back(Opcode instruction, size_t offset) const {
    const uint16_t distance = u8_to_u16(chunk->code.at(offset + 1), chunk->code.at(offset + 2));
    size_t destination = offset + 3 - distance;
    *out << fmt::format("{:18} {} -> {}\n", opcode_names.at(instruction), spaces_4, destination);
    return offset + 3;
}


size_t Disassembler::load_immediate(Opcode instruction, size_t offset) const {
    const size_t index = chunk->code.at(offset + 1);
    const Value value = Chunk::read_immediate(index);
    std::string value_str = LoxValue::to_visual_string(value);
    *out << fmt::format("{:18} {} value: {}\n", opcode_names.at(instruction), spaces_4, value_str);
    return offset + 2;
}

size_t Disassembler::constant_index_2(Opcode instruction, size_t offset) const {
    uint16_t index = u8_to_u16(chunk->code.at(offset + 1), chunk->code.at(offset + 2));
    const Value value = chunk->constants.at(index);
    std::string value_str = LoxValue::to_visual_string(value);
    *out << fmt::format("{:18} {} index: {}, value: {}\n", opcode_names.at(instruction), spaces_4, index, value_str);
    return offset + 3;
}

size_t Disassembler::constant_index_2_as_type(Opcode instruction, size_t offset) const {
    const uint16_t index = u8_to_u16(chunk->code.at(offset + 1), chunk->code.at(offset + 2));
    const Value value = chunk->constants.at(index);
    std::string value_str = LoxValue::to_visual_string(value);
    *out << fmt::format("{:18} {} type: '{}'\n", opcode_names.at(instruction), spaces_4, value_str);
    return offset + 3;
}

size_t Disassembler::instruction_identifier_operand_2(Opcode instruction, size_t offset) const {
    const uint16_t string_id = u8_to_u16(chunk->code.at(offset + 1), chunk->code.at(offset + 2));
    std::string identifier = StringIntern::read_from_id(string_id);
    *out << fmt::format("{:18} {} identifier: {}\n", opcode_names.at(instruction), spaces_4, identifier);
    return offset + 3;
}

size_t Disassembler::make_closure(Opcode instruction, size_t offset) const {
    uint8_t len = chunk->code.at(offset + 1);
    *out << fmt::format("{:18} {} num of captured: {}\n", opcode_names.at(instruction), spaces_4, len);
    return offset + 2 + 2 * len;
}

size_t Disassembler::make_class(Opcode instruction, size_t offset) const {
    const uint16_t string_id = u8_to_u16(chunk->code.at(offset + 1), chunk->code.at(offset + 2));
    std::string identifier = StringIntern::read_from_id(string_id);
    uint8_t num_field = chunk->code.at(offset + 3);
    uint8_t num_method = chunk->code.at(offset + 4);
    *out << fmt::format("{:18} {} class: {}; field {}, method {}\n", opcode_names.at(instruction), spaces_4, identifier, num_field, num_method);
    return offset + 5;
}

size_t Disassembler::method_bind(Opcode instruction, size_t offset) const {
    const uint16_t string_id = u8_to_u16(chunk->code.at(offset + 1), chunk->code.at(offset + 2));
    std::string identifier = StringIntern::read_from_id(string_id);
    *out << fmt::format("{:18} {} method: {}\n", opcode_names.at(instruction), spaces_4, identifier);
    return offset + 3;
}

size_t Disassembler::method_invoke(Opcode instruction, size_t offset) const {
    const uint16_t string_id = u8_to_u16(chunk->code.at(offset + 1), chunk->code.at(offset + 2));
    std::string identifier = StringIntern::read_from_id(string_id);
    uint8_t arg_count = chunk->code.at(offset + 3);
    *out << fmt::format("{:18} {} method: {}; arg count {}\n", opcode_names.at(instruction), spaces_4, identifier, arg_count);
    return offset + 4;
}


