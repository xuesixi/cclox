#include "vm.h"
#include "chunk.h"
#include "value.h"
#include "common.h"
#include "visual.h"
#include "compiler.h"
#include <iostream>
#include "objects/loxstring.h"
#include <variant>

VM::VM() {
    globals = std::make_shared<std::unordered_map<std::string, Value> >();
}

InterpreterResult VM::interpret(std::string &&source) {
    Compiler compiler;
    auto f = compiler.compile(std::move(source));
    if (f) {
        frames.push_back({f, 0, 0}); // 栈底部的第一个元素是main
        push(f);
        return run();
    } else {
        return InterpreterResult::CompileError;
    }
}

void VM::show_stack() {
    std::cout << "   ";
    for (size_t i = 0; i < stack.size(); i ++) {
        if (i == frames.back().fp) {
            Visual::print_with_color(fmt::format("[{}] ", Visual::to_visual_string(stack.at(i))), Color::RED);
        } else {
            Visual::print_with_color(fmt::format("[{}] ", Visual::to_visual_string(stack.at(i))), Color::BLUE);
        }
    }
    std::cout << std::endl;
}

InterpreterResult VM::run() {
    Disassembler disassembler;

    const bool trace = Flag::trace; // 据说保存为本地变量可以帮助编译器优化？

    try {
        while (true) {

            // 运行一条指令之前，输出一次栈和指令的信息，方便一步步看到整个运行的过程
            if (trace) {
                show_stack();
                // 如果栈帧发生变化，那么chunk也会变化。这里偷懒，在每次反汇编之前都重新设置一次，避免没有同步
                disassembler.set_chunk(&chunk());
                disassembler.disassemble_instruction(pc());
            }

            OpCode instruction = read_opcode();

            switch (instruction) {
                case OpCode::Return: {
                    // 获取栈顶的值作为返回值。
                    // 弹出最后一个栈帧。如果此时没有任何栈帧，说明main已经执行完毕。则结束
                    // 否则，缩减栈（清除上一个栈帧中的本地变量），然后将返回值添加在原本栈帧的fp处。
                    Value return_value = pop_and_get();
                    auto last_frame = frames.back();
                    frames.pop_back();
                    if (frames.empty()) {
                        stack.pop_back(); // 弹出main
                        return InterpreterResult::OK;
                    }
                    stack.resize(last_frame.fp);
                    stack.push_back(return_value);
                    break;
                }
                case OpCode::LoadConstant: {
                    Value value = read_constant_1();
                    push(std::move(value));
                    break;
                }
                case OpCode::LoadConstant2: {
                    Value value = read_constant_2();
                    push(std::move(value));
                    break;
                }
                case OpCode::LoadImmediate: {
                    uint8_t index = read_operand_1();
                    Value value = Chunk::read_immediate(index);
                    push(std::move(value));
                    break;
                }
                case OpCode::Negate: {
                    Value v = pop_and_get();
                    push(-v);
                    break;
                }
                case OpCode::Add: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a + b);
                    break;
                }
                case OpCode::Subtract: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a - b);
                    break;
                }
                case OpCode::Multipy: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a * b);
                    break;
                }
                case OpCode::Divide: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a / b);
                    break;
                }
                case OpCode::Power: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(LoxValue::power(a, b));
                    break;
                }
                case OpCode::LoadNil: {
                    push(nullptr);
                    break;
                }
                case OpCode::LoadTrue: {
                    push(true);
                    break;
                }
                case OpCode::LoadFalse: {
                    push(false);
                    break;
                }
                case OpCode::Not: {
                    Value v = pop_and_get();
                    push(!LoxValue::to_bool(v));
                    break;
                }
                case OpCode::Equal: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a == b);
                    break;
                }
                case OpCode::Greater: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a > b);
                    break;
                }
                case OpCode::Less: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a < b);
                    break;
                }
                case OpCode::Print: {
                    Value v = pop_and_get();
                    Visual::print_with_color(LoxValue::to_string(v) + "\n", Color::GREEN);
                    break;
                }
                case OpCode::Pop: {
                    pop();
                    break;
                }
                case OpCode::DefineGlobal: {
                    Value value = pop_and_get();
                    std::string key = read_identifier();
                    (*globals)[key] = value;
                    break;
                }
                case OpCode::LoadGlobal: {
                    std::string key = read_identifier();
                    auto found = globals->find(key);
                    if (found == globals->end()) {
                        throw LoxNameError(fmt::format("the variable: {} is not found", key));
                    } else {
                        push(found->second);
                    }
                    break;
                }
                case OpCode::SetGlobal: {
                    std::string key = read_identifier();
                    auto found = globals->find(key);
                    if (found == globals->end()) {
                        throw LoxNameError(fmt::format("the variable: {} is not found", key));
                    } else {
                        Value v = stack.back();
                        (*globals)[key] = v;
                    }
                    break;
                }
                case OpCode::LoadLocal: {
                    auto index = read_operand_1();
                    push(frame_at(index));
                    break;
                }
                case OpCode::SetLocal: {
                    auto index = read_operand_1();
                    frame_at(index) = stack.back();
                    break;
                }
                case OpCode::PopN: {
                    auto amount = read_operand_1();
                    stack.resize(stack.size() - amount);
                    break;
                }
                case OpCode::Jump: {
                    auto distance = read_operand_2();
                    pc() += distance;
                    break;
                }
                case OpCode::JumpIfPopFalse: {
                    auto distance = read_operand_2();
                    if (!LoxValue::to_bool(pop_and_get())) {
                        pc() += distance;
                    }
                    break;
                }
                case OpCode::JumpIfFalse: {
                    auto distance = read_operand_2();
                    if (!LoxValue::to_bool(stack.back())) {
                        pc() += distance;
                    }
                    break;
                }
                case OpCode::JumpBack: {
                    auto distance = read_operand_2();
                    pc() -= distance;
                    break;
                }
                case OpCode::Call: {
                    // fn, 1, 2
                    uint8_t arg_count = read_operand_1();
                    call_value(arg_count);
                    break;
                }
                default:
                    implementation_error(fmt::format("unknown opcode inside the vm running. code num: {}",
                                                     static_cast<uint8_t>(instruction)));
            }
        }
    } catch (LoxError &error) {
        std::cerr << error.what() << std::endl;
        return InterpreterResult::RuntimeError;
    } catch (CompilerError &error) {
        std::cerr << error.what() << std::endl;
        return InterpreterResult::RuntimeError;
    }
}

void VM::call_value(size_t arg_count) {
    size_t fp = stack.size() - 1 - arg_count;
    Value callable = stack.at(fp);
    auto test = LoxValue::to_reference<LoxFunction>(callable);
    if (test == nullptr) {
        throw LoxTypeError(fmt::format("the value {} cannot not be called", LoxValue::to_string(callable)));
    }
    auto f = LoxValue::to_reference_unsafe<LoxFunction>(callable);
    if (f->arity() != arg_count) {
        throw LoxArgError(fmt::format("the callable {} expect {} arguments, but got {}", LoxValue::to_string(f), f->arity(), arg_count));
    }
    CallFrame frame {f, 0, fp};
    frames.push_back(frame);
}
