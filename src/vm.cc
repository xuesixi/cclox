#include "vm.h"
#include "chunk.h"
#include "value.h"
#include <iostream>
#include <variant>
#include "common.h"
#include "vm_visualizer.h"

VM::VM(): pc(0) {}

InterpreterResult VM::interpret(std::shared_ptr<Chunk> chunk) {
    this->chunk = std::move(chunk); // todo: ok?
    this->pc = 0;
    return run();
}

InterpreterResult VM::run() {
    Disassembler disass(chunk);

    #ifdef DEBUG_TRACE_EXECUTION
        // 先把整个chunk反汇编一次，输出其结果
        disass.disassemble("test chunk");
    #endif

    while (true) {
        #ifdef DEBUG_TRACE_EXECUTION
            // 运行一条指令之前，再输出一次栈的样子和指令的信息，方便一步步看到整个运行的过程
            Visual::show_stack(*this);
            disass.disassemble_instruction(pc);

        #endif

        OpCode instruction = read_opcode();

        switch (instruction) {

            case OpCode::Return: {
                Value v = pop();
                // LoxValue::print(v);
                Visual::print_with_color(LoxValue::to_string(v), Color::YELLOW);
                std::cout << std::endl;
                return InterpreterResult::OK;
            }
            case OpCode::LoadConstant8: {
                Value value = read_constant_1();
                push(value);
                break;
            }
            case OpCode::LoadConstant16: {
                Value value = read_constant_2();
                push(value);
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

        }
    }
}