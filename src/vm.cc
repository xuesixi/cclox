#include "vm.h"
#include "chunk.h"
#include "value.h"
#include "common.h"
#include "visual.h"
#include "compiler.h"
#include <sstream>
#include <iostream>
#include <string>
#include "objects/loxstring.h"
#include <variant>

VM::VM() {
    globals = std::make_shared<std::unordered_map<std::string, Value> >();
}

InterpreterResult VM::interpret(std::string &&source) {
    Compiler compiler;
    auto f = compiler.compile(std::move(source));
    auto closure = LoxObject::allocate_as<LoxClosure>(f);
    closure->fix_size();
    if (f) {
        frames.push_back({closure, 0, 0}); // 栈底部的第一个元素是main
        push(closure);
        return run();
    } else {
        return InterpreterResult::CompileError;
    }
}

void VM::show_stack() {
    Visual::print_with_color(fmt::format(" heap: {}  ", runtime.allocated_size.load()), Color::MAGENTA);
    for (size_t i = 0; i < stack.size(); i++) {
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

            Opcode instruction = read_opcode();

            switch (instruction) {
                case Opcode::Return: {
                    // 获取栈顶的值作为返回值。
                    // 弹出最后一个栈帧。如果此时没有任何栈帧，说明main已经执行完毕。则结束
                    // 否则，缩减栈（清除上一个栈帧中的本地变量），然后将返回值添加在原本栈帧的fp处。
                    Value return_value = pop_and_get();
                    auto last_frame = frames.back();

                    escape_above(last_frame.fp); // 逃逸该栈帧中的被捕获值

                    frames.pop_back();
                    if (frames.empty()) {
                        stack.pop_back(); // 弹出main
                        return InterpreterResult::OK;
                    }
                    stack.resize(last_frame.fp);
                    stack.push_back(return_value);
                    break;
                }
                case Opcode::LoadConstant: {
                    Value value = read_constant_1();
                    push(std::move(value));
                    break;
                }
                case Opcode::LoadConstant2: {
                    Value value = read_constant_2();
                    push(std::move(value));
                    break;
                }
                case Opcode::LoadImmediate: {
                    uint8_t index = read_operand_1();
                    Value value = Chunk::read_immediate(index);
                    push(std::move(value));
                    break;
                }
                case Opcode::Negate: {
                    Value v = pop_and_get();
                    push(-v);
                    break;
                }
                case Opcode::Add: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a + b);
                    break;
                }
                case Opcode::Subtract: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a - b);
                    break;
                }
                case Opcode::Multipy: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a * b);
                    break;
                }
                case Opcode::Divide: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a / b);
                    break;
                }
                case Opcode::Power: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(LoxValue::power(a, b));
                    break;
                }
                case Opcode::LoadNil: {
                    push(nullptr);
                    break;
                }
                case Opcode::LoadTrue: {
                    push(true);
                    break;
                }
                case Opcode::LoadFalse: {
                    push(false);
                    break;
                }
                case Opcode::Not: {
                    Value v = pop_and_get();
                    push(!LoxValue::to_bool(v));
                    break;
                }
                case Opcode::Equal: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a == b);
                    break;
                }
                case Opcode::Greater: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a > b);
                    break;
                }
                case Opcode::Less: {
                    Value b = pop_and_get();
                    Value a = pop_and_get();
                    push(a < b);
                    break;
                }
                case Opcode::Print: {
                    Value v = pop_and_get();
                    Visual::print_with_color(LoxValue::to_string(v) + "\n", Color::GREEN);
                    break;
                }
                case Opcode::Pop: {
                    pop();
                    break;
                }
                case Opcode::DefineGlobal: {
                    Value value = pop_and_get();
                    std::string key = read_identifier();
                    (*globals)[key] = value;
                    break;
                }
                case Opcode::LoadGlobal: {
                    std::string key = read_identifier();
                    auto found = globals->find(key);
                    if (found == globals->end()) {
                        throw LoxNameError(fmt::format("the variable: {} is not found", key));
                    } else {
                        push(found->second);
                    }
                    break;
                }
                case Opcode::SetGlobal: {
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
                case Opcode::LoadLocal: {
                    auto index = read_operand_1();
                    push(frame_at(index));
                    break;
                }
                case Opcode::SetLocal: {
                    auto index = read_operand_1();
                    frame_at(index) = stack.back();
                    break;
                }
                case Opcode::PopN: {
                    auto amount = read_operand_1();
                    stack.resize(stack.size() - amount);
                    break;
                }
                case Opcode::Jump: {
                    auto distance = read_operand_2();
                    pc() += distance;
                    break;
                }
                case Opcode::JumpIfPopFalse: {
                    auto distance = read_operand_2();
                    if (!LoxValue::to_bool(pop_and_get())) {
                        pc() += distance;
                    }
                    break;
                }
                case Opcode::JumpIfFalse: {
                    auto distance = read_operand_2();
                    if (!LoxValue::to_bool(stack.back())) {
                        pc() += distance;
                    }
                    break;
                }
                case Opcode::JumpBack: {
                    auto distance = read_operand_2();
                    pc() -= distance;
                    break;
                }
                case Opcode::Call: {
                    // fn, 1, 2
                    uint8_t arg_count = read_operand_1();
                    call_value(arg_count);
                    break;
                }
                case Opcode::MakeClosure: {
                    Value value = pop_and_get();
                    auto fun = LoxValue::to_reference_unsafe<LoxFunction>(value);
                    auto new_closure = LoxObject::allocate_as<LoxClosure>(fun);
                    uint8_t num_captures = read_operand_1();
                    for (uint8_t i = 0; i < num_captures; i++) {
                        auto is_local = read_operand_1();
                        auto index = read_operand_1();
                        if (is_local) {
                            // 如果是本地变量，那么需要捕获
                            new_closure->captureds.push_back(capture_value(index));
                        } else {
                            // 如果是本身就是upvalue，说明外层已经捕获过了，直接复制
                            new_closure->captureds.push_back(closure()->captureds.at(index));
                        }
                    }
                    new_closure->fix_size();
                    push(new_closure);
                    break;
                }
                case Opcode::LoadCaptured: {
                    auto index = read_operand_1();
                    Value v = closure()->captureds.at(index)->value();
                    push(v);
                    break;
                }
                case Opcode::SetCaptured: {
                    auto index = read_operand_1();
                    closure()->captureds.at(index)->value() = stack.back();
                    break;
                }
                case Opcode::Recur: {
                    auto arg_count = read_operand_1();
                    recur_call(arg_count);
                    break;
                }
                case Opcode::StringConcat: {
                    auto count = read_operand_1();
                    std::stringstream s;
                    for (uint8_t i = 0; i < count; i ++) {
                        // a, b, c, x
                        Value v = stack.at(stack.size() - count + i);
                        s << LoxValue::to_string(v);
                    }
                    stack.resize(stack.size() - count);
                    LoxReference result = LoxObject::allocate_as_ref<LoxString>(s.str());
                    result->fix_size();
                    push(result);
                    break;
                }
                default:
                    implementation_error(fmt::format("unknown opcode inside the vm running. code num: {}",
                                                     static_cast<uint8_t>(instruction)));
            }
        }
    } catch (LoxError &error) {
        std::cerr << error.what() << std::endl;
        std::cerr << backtrace();
        return InterpreterResult::RuntimeError;
    } catch (CompilerError &error) {
        std::cerr << error.what() << std::endl;
        return InterpreterResult::RuntimeError;
    }
}

std::string VM::backtrace() {
    std::stringstream result;
    for (auto it = frames.rbegin(); it != frames.rend(); it ++) {
        auto call_frame = *it;
        auto line = call_frame.closure->function->get_chunk().get_line_num(call_frame.pc);
        result << fmt::format("{} at line {}\n", call_frame.closure->to_string(), line);
    }
    return result.str();
}

void VM::call_value(size_t arg_count) {
    if (frames.size() == Configuration::frame_max) {
       throw LoxStackOverflowError("stack overflow");
    }
    size_t fp = stack.size() - 1 - arg_count;
    Value callable = stack.at(fp);
    if (LoxValue::try_cast<LoxClosure>(callable) == nullptr) {
        throw LoxTypeError(fmt::format("the value {} is not a function thus cannot not be called", LoxValue::to_string(callable)));
    }
    auto cl = LoxValue::to_reference_unsafe<LoxClosure>(callable);
    if (cl->function->arity() != arg_count) {
        throw LoxArgError(fmt::format("the callable {} expect {} arguments, but got {}", LoxValue::to_string(cl),
                                      cl->function->arity(), arg_count));
    }
    CallFrame frame{cl, 0, fp};
    frames.push_back(frame);
}

void VM::recur_call(size_t arg_count) {
    if (arg_count != closure()->function->arity()) {
        throw LoxArgError(fmt::format("the callable {} expect {} arguments, but got {}", LoxValue::to_string(closure()),
                                      closure()->function->arity(), arg_count));
    }
    for (uint8_t i = 0; i < arg_count; i++) {
        frame_at(1 + i) = stack.at(stack.size() - arg_count + i);
    }
    stack.resize(frame().fp + arg_count + 1);
    pc() = 0;
}
