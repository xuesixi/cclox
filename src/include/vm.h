
#ifndef CCLOX_VM_H
#define CCLOX_VM_H

#include "cclox_util.h"
#include "captured.h"
#include "objects/loxstring.h"
#include "chunk.h"
#include "value.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "disassembler.h"
#include "objects/loxfunction.h"
#include "objects/loxclosure.h"

constexpr int STACK_MAX = 256;

class VM;

enum class InterpreterResult {
    OK,
    CompileError,
    RuntimeError,
};

struct CallFrame {
    // std::shared_ptr<LoxFunction> function_;
    std::shared_ptr<LoxClosure> closure;
    size_t pc;
    size_t fp; // frame pointer, 帧指针，本帧的起始处
};

class VM {
public:
    explicit VM();

    VM(const VM &other) = delete;

    VM(VM &&other) = delete;

    VM &operator=(const VM &other) = delete;

    VM &operator=(VM &&other) = delete;

    InterpreterResult interpret(std::string &&source);

    void show_stack();

private:
    InterpreterResult run();

    void push(Value &&value) {
        stack.push_back(std::move(value));
    }

    void push(const Value &value) {
        stack.push_back(value);
    }

    [[nodiscard]] Value pop_and_get() {
        Value value = stack.back();
        stack.pop_back();
        return value;
    }

    void pop() {
        stack.pop_back();
    }

    // 读取下一个指令
    OpCode read_opcode() {
        return static_cast<OpCode>(next());
    }

    // 读取下一个操作数
    uint8_t read_operand_1() {
        return next();
    }

    // 读取后两个操作数，将它们解释为一个uint16。先读取的是low，后读取的是high
    uint16_t read_operand_2() {
        uint8_t low = next();
        uint8_t high = next();
        return u8_to_u16(low, high);
    }

    // 读取下一个操作数作为索引，从常数池中读取对应的值
    Value read_constant_1() {
        uint8_t index = read_operand_1();
        return chunk().constant_at(index);
    }

    // 读取下两个操作数作为uint16索引，从常数池中读取对应的值
    Value read_constant_2() {
        uint16_t index = read_operand_2();
        return chunk().constant_at(index);
    }

    // 读取下两个操作数作为uint16索引，从标识符池中读取对应的标识符
    std::string read_identifier() {
        uint16_t key = read_operand_2();
        return chunk().read_identifier(key);
    }

    std::shared_ptr<LoxClosure> &closure() {
        return frames.back().closure;
    }

    CallFrame &frame() {
        return frames.back();
    }

    Chunk &chunk() {
        return frame().closure->function_->get_chunk();
    }

    size_t &pc() {
        return frame().pc;
    }

    uint8_t next() {
        return chunk().code_at(pc()++);
    }

    /**
     * 返回相对于本栈帧底距离为index的值
     */
    Value &frame_at(uint8_t index) {
        return stack.at(frame().fp + index);
    }

    /**
     * 捕获栈上位于local_index上的那个本地变量，将其添加到open_captured中
     * @param local_index 被捕获的本地变量的本地索引
     * @return 新生成的捕获值
     */
    std::shared_ptr<Captured> capture_value(uint8_t local_index) {
        auto new_captured = std::make_shared<Captured>(stack, frame().fp + local_index);
        open_captured.push_back(new_captured);
        return new_captured;
    }

    /**
     * 将位处于index及以上的所有open的捕获值进行逃逸
     * @param index 该栈索引以及其上的所有捕获值将会被逃逸
     */
    void escape_above(size_t index) {
        while (open_captured.empty() == false) {
            auto curr = open_captured.back();
            if (curr->index() >= index) {
                open_captured.pop_back();
                curr->escape();
            } else {
                break;
            }
        }
    }

    /**
     * 根据arg_count的值计算被调用者的位置，调用之，产生新的栈帧。
     */
    void call_value(size_t arg_count);

    std::shared_ptr<std::unordered_map<std::string, Value> > globals; // 多个虚拟机线程共享同一个全局变量池
    std::vector<CallFrame> frames;
    std::vector<Value> stack; // 栈
    std::vector<std::shared_ptr<Captured>> open_captured; // 仍然存在于栈上的捕获值
};

#endif
