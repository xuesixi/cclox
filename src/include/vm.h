
#ifndef CCLOX_VM_H
#define CCLOX_VM_H

#include "cclox_util.h"
#include "objects/loxcaptured.h"
#include "chunk.h"
#include "value.h"
#include <memory>
#include <mutex>
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
    explicit VM() {
        std::lock_guard lock{Runtime::gc_mutex};
        vm_list_it = Runtime::vm_list.insert(Runtime::vm_list.end(), this);
    }

    ~VM() {
        check_gc();
        std::lock_guard lock{Runtime::gc_mutex};
        Runtime::vm_list.erase(vm_list_it);
    }

    VM(const VM &other) = delete;

    VM(VM &&other) = delete;

    VM &operator=(const VM &other) = delete;

    VM &operator=(VM &&other) = delete;

    /**
     * 用传入的源代码编译出一个closure对象，将其的对应栈帧置入栈中。然后开始运行。
     * 全局变量不属于单个vm的成员，因此多次interpret时全局变量的信息会继承。
     * @param source 要运行的源代码
     * @return 运行结果
     */
    InterpreterResult interpret(std::string &&source);

    /**
     * 根据栈来执行指令。运行直到帧栈为空。该函数内部会捕获LoxError
     * @return 运行结果
     */
    InterpreterResult run();

    /**
     * 打印展示当前的栈状态
     */
    void show_stack();

    std::shared_ptr<LoxClosure> &closure() {
        return frames_.back().closure;
    }

    CallFrame &curr_frame() {
        return frames_.back();
    }

    Chunk &chunk() {
        return curr_frame().closure->function->get_chunk();
    }

    size_t &pc() {
        return curr_frame().pc;
    }

    uint8_t next() {
        return chunk().code_at(pc()++);
    }

    std::vector<Value> &stack() {
        return stack_;
    }

    std::vector<CallFrame> &frames() {
        return frames_;
    }

    /**
     * 返回相对于本栈帧底距离为index的值
     */
    Value &frame_at(uint8_t index) {
        return stack_.at(curr_frame().fp + index);
    }

    /**
     * 用指定的closure创建一个新的栈帧。
     * @param cl 新的栈帧的内容函数
     * @param fp 新的栈帧的栈底位置
     */
    void setup_frame(const std::shared_ptr<LoxClosure> &cl, size_t fp) {
        frames_.push_back({cl, 0, fp});
    }

private:

    void push(Value &&value) {
        stack_.push_back(std::move(value));
    }

    void push(const Value &value) {
        stack_.push_back(value);
    }

    [[nodiscard]] Value pop_and_get() {
        Value value = stack_.back();
        stack_.pop_back();
        return value;
    }

    void pop() {
        stack_.pop_back();
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

    /**
     * 捕获栈上位于local_index上的那个本地变量，将其添加到open_captured中
     * @param local_index 被捕获的本地变量的本地索引
     * @return 新生成的捕获值
     */
    std::shared_ptr<Captured> capture_value(uint8_t local_index) {
        auto new_captured = Runtime::allocate_as<Captured>(stack_, curr_frame().fp + local_index);
        Runtime::record_allocation(new_captured);
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
     * 在安全点执行该函数，检查是否要gc。
     */
    void check_gc();

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
     * 从该接收者的类中查找对应的方法
     * @param receiver 该方法要绑定的接受者
     * @param string_id 方法名的string id
     */
    std::shared_ptr<LoxClosure> method_lookup(const Value &receiver, OperandSize string_id);

    /**
     * 查找一个closure，根据arg_count计算新的fp，创建对应的栈帧。将新栈帧的底部替换为接受者
     */
    void method_invoke(OperandSize string_id, uint8_t arg_count);

    std::vector<CallFrame> frames_;
    std::vector<Value> stack_; // 栈
    std::vector<std::shared_ptr<Captured> > open_captured; // 仍然存在于栈上的捕获值
    std::list<VM*>::iterator vm_list_it;
};

#endif
