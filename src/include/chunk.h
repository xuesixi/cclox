//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_CHUNK_H
#define CCLOX_CHUNK_H

#include <utility>
#include <vector>
#include "value.h"
#include "cclox_util.h"
#include "error.h"

class Disassembler;

enum class OpCode: uint8_t {
    Return,
    LoadConstant,
    LoadConstant2,
    LoadImmediate,
    Negate,
    Add,
    Subtract,
    Multipy,
    Divide,
    LoadNil,
    LoadTrue,
    LoadFalse,
    Not,
    Greater,
    Less,
    Equal,
    Print,
    Pop,
    DefineGlobal,
    LoadGlobal,
    SetGlobal,
};

using OperandSize = uint16_t;

class Chunk {
public:
    friend class Disassembler;


    uint8_t code_at(size_t index) {
        return code.at(index);
    }

    Value constant_at(size_t index) {
        return constants.at(index);
    }

    /**
     * 向code中写入一个新的指令，并记录其所在的行数
     */
    void write_opcode(OpCode opcode, int line) {
        code.push_back(static_cast<uint8_t>(opcode));
        lines.push_back(line);
    }

    /**
     * 向code中写入operand所代表的字节。如果在uint8范围内，写入一个字节，如果超出此范围，但处在uint16范围内，写入两个字节。否则abort
     */
    void write_operand(OperandSize operand, int line) {
        if (within<uint8_t>(operand)) {
            code.push_back(static_cast<uint8_t>(operand));
            lines.push_back(line);
        } else if (within<uint16_t>(operand)){
            write_operand_2(operand, line);
        } else {
            implementation_error("not within uint16 limits");
        }
    }

    /**
     * 向code中写入operand。无论operand的值是否超出uint8的范围，都会写入两个字节
     */
    void write_operand_2(OperandSize operand, int line) {
        auto [high, low] = u16_to_u8(operand);
        code.push_back(low);
        code.push_back(high);
        lines.push_back(line);
        lines.push_back(line);
    }

    /**
     * 向常数池中增加一个值，并返回其索引。如果索引在uint16范围内，返回之。否则，抛出ConstantPoolOverflowError
     */
    OperandSize add_constant(Value &&value) {
        constants.push_back(std::move(value));
        size_t index = constants.size() - 1;
        if (!within<OperandSize>(index)) {
            throw ConstantPoolOverflowError("constant pool overflow");
        } else {
            return index;
        }
    }

    /**
     * 获取str在标识符中的键。如果已存在，则直接返回键。否则，向标识符池中增加一个值，如果键在uint16范围内，返回之。否则，抛出IdentifierPoolOverflowError
     */
    OperandSize add_identifier(const std::string &str) {

        auto found = name_to_key.find(str);
        if (found != name_to_key.end()) {
            // 如果存在，则返回键
            return found->second;
        }

        // 如果不存在，则新增
        OperandSize key = key_to_name.size();
        if (!within<uint16_t>(key)) {
            throw IdentifierPoolOverFlowError("identifier pool overflow");
        }
        key_to_name[key] = str;
        name_to_key[str] = key;
        return key;
    }

    std::string read_identifier(OperandSize key) {
        return key_to_name[key];
    }

    /**
     * 对于一个Value，如果它属于立即数，返回其立即数索引，否则返回255
     */
    static uint8_t to_immediate(Value value);

    /**
     * 给定一个立即数索引，返回其对应的Value
     */
    static Value read_immediate(uint8_t index);


private:
    // 字节码
    std::vector<uint8_t> code;
    // 常数池
    std::vector<Value> constants;
    // 与字节码一一对应的行数记录
    std::vector<int> lines;
    // 标识符池
    std::unordered_map<OperandSize, std::string> key_to_name;
    std::unordered_map<std::string, OperandSize> name_to_key;
};

#endif //CCLOX_CHUNK_H
