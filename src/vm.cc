#include "vm.h"
#include "chunk.h"
#include "value.h"
#include "common.h"
#include "vm_visualizer.h"
#include "compiler.h"
#include <iostream>
#include "objects/loxstring.h"
#include <variant>

VM::VM(): pc(0) {}

InterpreterResult VM::interpret(std::string &&source) {
    this->pc = 0;
    Compiler compiler;
    chunk = compiler.compile(std::move(source));
    if (chunk) {
        return run();
    } else {
        return InterpreterResult::CompileError;
    }
}

InterpreterResult VM::run() {
    Disassembler disassembler(chunk);

    #ifdef DEBUG_TRACE_EXECUTION
        // 先把整个chunk反汇编一次，输出其结果
        disassembler.disassemble("test chunk");
    #endif

    try {
        while (true) {
#ifdef DEBUG_TRACE_EXECUTION
            // 运行一条指令之前，再输出一次栈的样子和指令的信息，方便一步步看到整个运行的过程
            Visual::show_stack(*this);
            disassembler.disassemble_instruction(pc);

#endif

            OpCode instruction = read_opcode();

            switch (instruction) {

                case OpCode::Return: {
                    Value v = pop();
                    Visual::print_with_color(LoxValue::to_string(v), Color::YELLOW);
                    std::cout << std::endl;
                    return InterpreterResult::OK;
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
                    Value v = pop();
                    push(-v);
                    break;
                }
                case OpCode::Add: {
                    Value b = pop();
                    Value a = pop();
                    push(a + b);
                    break;
                }
                case OpCode::Subtract: {
                    Value b = pop();
                    Value a = pop();
                    push(a - b);
                    break;
                }
                case OpCode::Multipy: {
                    Value b = pop();
                    Value a = pop();
                    push(a * b);
                    break;
                }
                case OpCode::Divide: {
                    Value b = pop();
                    Value a = pop();
                    push(a / b);
                    break;
                }
                case OpCode::LoadNil: {
                    push(nullptr);
                    break;
                };
                case OpCode::LoadTrue: {
                    push(true);
                    break;
                }
                case OpCode::LoadFalse: {
                    push(false);
                    break;
                }
                case OpCode::Not: {
                    Value v = pop();
                    push(!LoxValue::to_bool(v));
                    break;
                }
                case OpCode::Equal: {
                    Value b = pop();
                    Value a = pop();
                    push(a == b);
                    break;
                }
                case OpCode::Greater: {
                    Value b = pop();
                    Value a = pop();
                    push(a > b);
                    break;
                }
                case OpCode::Less: {
                    Value b = pop();
                    Value a = pop();
                    push(a < b);
                    break;
                }
                case OpCode::Print: {
                    Value v = pop();
                    Visual::print_with_color(LoxValue::to_string(v) + "\n", Color::GREEN);
                    break;
                }
                case OpCode::Pop: {
                    pop();
                    break;
                }
                case OpCode::DefineGlobal: {
                    Value value = pop();
                    std::string key = read_identifier();
                    globals[key] = value;
                    break;
                }
                case OpCode::LoadGlobal: {
                    std::string key = read_identifier();
                    auto found = globals.find(key);
                    if (found == globals.end()) {
                        throw LoxNameError(fmt::format("the variable: {} is not found", key));
                    } else {
                        push(found->second);
                    }
                    break;
                }
                default:
                    implementation_error(fmt::format("unknown opcode inside the vm running. code num: {}", static_cast<uint8_t>(instruction)));
            }
        }
    } catch (LoxError &error) {
        std::cerr << error.what() << std::endl;
        return InterpreterResult::RuntimeError;
    } catch (InterpreterError &error) {
        std::cerr << error.what() << std::endl;
        return InterpreterResult::RuntimeError;
    }
}