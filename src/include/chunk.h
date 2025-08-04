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

class VM;

class Disassembler;

enum class OpCode: uint8_t {
    Return,
    LoadConstant8,
    LoadConstant16,
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
};

class Chunk {
public:
    friend class VM;
    friend class Disassembler;


    /**
     * 向code中写入一个新的指令，并记录其所在的行数
     */
    void write_opcode(OpCode opcode, int line) {
        code.push_back(static_cast<uint8_t>(opcode));
        lines.push_back(line);
    }

    /**
     * 向code中写入operand所代表的字节。如果在uint8范围内，写入一个字节，如果超出此范围，但处在uint16范围内，写入两个字节。否则编译异常。
     */
    void write_operand(size_t operand, int line) {
        if (within<uint8_t>(operand)) {
            code.push_back(static_cast<uint8_t>(operand));
            lines.push_back(line);
        } else if (within<uint16_t>(operand)){
            auto [low, high] = u16_to_u8(operand);
            code.push_back(low);
            code.push_back(high);
            lines.push_back(line);
            lines.push_back(line);
        } else {
            std::abort();
        }
    }

    /**
     * 向常数池中增加一个值，并返回其索引。该返回值可能大于uint8的极限。
     */
    size_t add_constant(Value &&value) {
        constants.push_back(std::move(value));
        return constants.size() - 1;
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
};

#endif //CCLOX_CHUNK_H
