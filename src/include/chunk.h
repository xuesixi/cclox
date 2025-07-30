//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_CHUNK_H
#define CCLOX_CHUNK_H

#include <utility>
#include <vector>
#include <limits>
#include "value.h"
#include "cclox_util.h"
#include "error.h"

class VM;

class Disassembler;

enum class OpCode: uint8_t {
    Return,
    LoadConstant8,
    LoadConstant16,
    Negate,
    Add,
    Subtract,
    Multipy,
    Divide,
};

class Chunk {
public:
    friend class VM;
    friend class Disassembler;

    /**
     * 向code中写入一个新的指令，并记录其所在的行数
     */
    void write_opcode(OpCode byte, int line) {
        code.push_back(static_cast<uint8_t>(byte));
        lines.push_back(line);
    }

    /**
     * 向code中写入index所代表的字节。如果在uint8范围内，写入一个字节，如果超出此范围，但处在uint16范围内，写入两个字节。否则编译异常。
     */
    void write_index(size_t index, int line) {
        if (index <= std::numeric_limits<uint8_t>::max()) {
            code.push_back(static_cast<uint8_t>(index));
            lines.push_back(line);
        } else if (index <= std::numeric_limits<uint16_t>::max()){
            auto [low, high] = u16_to_u8(index);
            code.push_back(low);
            code.push_back(high);
            lines.push_back(line);
            lines.push_back(line);
        } else {
            compile_error("constant pool overflow: too many constants!");
            std::abort();
        }
    }

    /**
     * 向常数池中增加一个值，并返回其索引
     */
    size_t add_constant(Value value) {
        constants.push_back(value);
        return constants.size() - 1;
    }

private:
    // 字节码
    std::vector<uint8_t> code;
    // 常数池
    std::vector<Value> constants;
    // 与字节码一一对应的行数记录
    std::vector<int> lines;
};

#endif //CCLOX_CHUNK_H
