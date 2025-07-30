
#ifndef CCLOX_VM_H
#define CCLOX_VM_H

#include "cclox_util.h"
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

    // 读取下一个指令
    OpCode read_opcode() {
        return static_cast<OpCode>(chunk->code.at(pc++));
    }
    // 读取下一个操作数
    uint8_t read_operand_1() {
        return chunk->code.at(pc++);
    }
    // 读取后两个操作数，将它们解释为一个uint16
    uint16_t read_operand_2() {
        uint8_t low = chunk->code.at(pc++);
        uint8_t high = chunk->code.at(pc++);
        return u8_to_u16(low, high);
    }
    // 读取下一个操作数作为索引，从常数池中读取对应的值
    Value read_constant_1() {
        uint8_t index = read_operand_1();
        return chunk->constants.at(index);
    }
    // 读取下两个操作数作为uint16索引，从常数池中读取对应的值
    Value read_constant_2() {
        uint16_t index = read_operand_2();
        return chunk->constants.at(index);
    }

    std::shared_ptr<Chunk> chunk;
    std::vector<Value> stack; // 栈
    size_t pc; // 下一个要执行的指令在字节码vector中的索引
};

#endif