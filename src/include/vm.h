
#ifndef CCLOX_VM_H
#define CCLOX_VM_H

#include "bytes.h"
#include "chunk.h"
#include "value.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "disassembler.h"

constexpr int STACK_MAX = 256;

namespace Visual {
    void show_stack(VM &vm);
}

enum class InterpreterResult {
    OK,
    CompileError,
    RuntimeError,
};

class VM {
public:
    explicit VM();
    VM(const VM &other) = delete;
    VM(VM &&other) = delete;
    VM & operator=(const VM &other) = delete;
    VM & operator=(VM &&other) = delete;
    InterpreterResult interpret(std::shared_ptr<Chunk> chunk);

    friend void Visual::show_stack(VM &vm);
private:

    InterpreterResult run();

    void push(Value value) {
        stack.push_back(value);
    }
    Value pop() {
        Value value = stack.back();
        stack.pop_back();
        return value;
    }

    OpCode read_opcode() {
        return static_cast<OpCode>(chunk->code.at(pc++));
    }
    uint8_t read_operand_1() {
        return chunk->code.at(pc++);
    }
    uint16_t read_operand_2() {
        uint8_t low = chunk->code.at(pc++);
        uint8_t high = chunk->code.at(pc++);
        return u8_to_u16(low, high);
    }
    Value read_constant_1() {
        uint8_t index = read_operand_1();
        return chunk->constants.at(index);
    }
    Value read_constant_2() {
        uint16_t index = read_operand_2();
        return chunk->constants.at(index);
    }


    std::shared_ptr<Chunk> chunk;
    std::vector<Value> stack;
    size_t pc; // index of the next-to-run instruction in the vector of bytecodes
};

#endif