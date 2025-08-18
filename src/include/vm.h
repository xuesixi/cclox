
#ifndef CCLOX_VM_H
#define CCLOX_VM_H

#include "cclox_util.h"
#include "captured.h"
#include "chunk.h"
#include "value.h"
#include <memory>
#include <vector>
#include "runtime.h"
#include "objects/loxfunction.h"
#include "objects/loxclosure.h"
#include "objects/loxmethod.h"

constexpr int STACK_MAX = 256;

class VM;

enum class InterpreterResult {
    OK,
    CompileError,
    RuntimeError,
};

struct CallFrame {
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

    /**
     * 用传入的源代码编译出一个closure对象，将其的对应栈帧置入栈中。然后开始运行。
     * 全局变量是一个vm的成员，因此多次interpret时全局变量的信息会继承。
     * @param source 要运行的源代码
     * @return 运行结果
     */
    InterpreterResult interpret(std::string &&source);

    /**
     * 打印展示当前的栈状态
     */
    void show_stack();

private:
    /**
     * 根据栈来执行指令。运行直到帧栈为空
     * @return 运行结果
     */
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
    Opcode read_opcode() {
        return static_cast<Opcode>(next());
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

    /**
     * 读取下一个操作数作为cache的索引，从缓存池中读取对应的缓存
     */
    Chunk::MethodCache &read_cache() {
        auto index = read_operand_1();
        return chunk().read_cache(index);
    }

    std::shared_ptr<LoxClosure> &closure() {
        return frames.back().closure;
    }

    CallFrame &frame() {
        return frames.back();
    }

    Chunk &chunk() {
        return frame().closure->function->get_chunk();
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
     * @return 一个代表帧栈的字符串。新的栈帧会出现在字符串的前面。
     */
    std::string backtrace();

    /**
     * 根据arg_count的值计算被调用者的位置，调用之，产生新的栈帧。
     * @throws LoxArgError 如果传入参数的数量与栈帧对应的closure不匹配
     */
    void call_value(size_t arg_count);

    void call_closure(size_t arg_count);

    void call_method(size_t arg_count);

    void call_class(size_t arg_count);

    void call_native(size_t arg_count);

    /**
     * 将当前栈帧末尾的arg_count个参数移动到栈帧的开头，删除除了参数之外的其他本地变量，pc归零。
     * @throws LoxArgError 如果传入参数的数量与栈帧对应的closure不匹配
     */
    void recur_call(size_t arg_count);

    /**
     * 查找一个closure，将其绑定到receiver上，产生一个method，将其入栈。
     * 该函数会先试图使用缓存，如果失败，则用哈希表查找并更新缓存
     * @param receiver 该方法要绑定的接受者
     * @param identifier_key 如果缓存失效，使用该key进行标识符查询。
     * @param cache 缓存
     */
    void method_lookup(const Value &receiver, OperandSize identifier_key, Chunk::MethodCache &cache);

    std::vector<CallFrame> frames;
    std::vector<Value> stack; // 栈
    std::vector<std::shared_ptr<Captured> > open_captured; // 仍然存在于栈上的捕获值
};

#endif
