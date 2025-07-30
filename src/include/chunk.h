//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_CHUNK_H
#define CCLOX_CHUNK_H

#include <utility>
#include <vector>
#include <limits>
#include "value.h"
#include "bytes.h"
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
    void write_opcode(OpCode byte, int line) {
        code.push_back(static_cast<uint8_t>(byte));
        lines.push_back(line);
    }

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

    size_t add_constant(Value value) {
        constants.push_back(value);
        return constants.size() - 1;
    }

private:

    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines;
};

#endif //CCLOX_CHUNK_H
