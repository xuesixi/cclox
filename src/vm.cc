#include "vm.h"
#include "chunk.h"
#include "value.h"
#include "common.h"
#include "compiler.h"
#include "disassembler.h"
#include "typechecker/st_runtime.h"
#include <sstream>
#include <iostream>
#include <variant>

#include "typechecker/ast.h"
#include "objects/loxstring.h"
#include "objects/loxclass.h"

#include "stringintern.h"
#include "objects/loxinstance.h"

std::atomic<int> VM::next_vm_id = 0;

InterpreterResult VM::interpret(std::string &&source) {
    try {
        Compiler compiler;
        auto f = compiler.compile(std::move(source));
        auto cl = Runtime::allocate_as<LoxClosure>(f);
        if (f && !Flag::not_run) {
            setup_frame(cl, 0);
            push(cl);
            Runtime::record_allocation(cl);
            Runtime::allow_gc = true;
            return run();
        } else {
            return InterpreterResult::CompileError;
        }

    } catch (ScannerError &error) {
        std::cerr << error.what() << std::endl;
        return InterpreterResult::CompileError;
    }
}

InterpreterResult VM::interpret_st(std::string &&source) {
    try {
        AstCompiler compiler;
        auto cl = compiler.compile(std::move(source));
        if (cl && !Flag::not_run) {
            setup_frame(cl, 0);
            push(cl);
            Runtime::record_allocation(cl);
            Runtime::allow_gc = true;
            return run();
        } else {
            return InterpreterResult::CompileError;
        }

    } catch (ScannerError &error) {
        std::cerr << error.what() << std::endl;
        return InterpreterResult::CompileError;
    }
}

void VM::show_stack() {
    print_log(fmt::format("@{} [no.{}] heap: {}  ", nanos_str(), vm_id, Runtime::allocated_size.load()), Color::MAGENTA);
    for (size_t i = 0; i < stack_.size(); i++) {
        if (i == frames_.back().fp) {
            print_log(fmt::format("[{}] ", LoxValue::to_visual_string(stack_.at(i))), Color::RED);
        } else {
            print_log(fmt::format("[{}] ", LoxValue::to_visual_string(stack_.at(i))), Color::BLUE);
        }
    }
    print_log("\n", Color::None);
    // std::cout << std::endl;
}

InterpreterResult VM::run() {
    Disassembler disassembler;
    if (log_stream.has_value()) {
        disassembler.set_out(&log_stream.value());
    } else {
        disassembler.set_out(&std::cout);
    }

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

                    Runtime::wait_sync();
                    Runtime::sync_flag.clear();

                    // 获取栈顶的值作为返回值。
                    // 弹出最后一个栈帧。如果此时没有任何栈帧，说明main已经执行完毕。则结束
                    // 否则，缩减栈（清除上一个栈帧中的本地变量），然后将返回值添加在原本栈帧的fp处。

                    Value return_value = pop_and_get();
                    auto last_frame = frames_.back();

                    escape_above(last_frame.fp); // 逃逸该栈帧中的被捕获值

                    frames_.pop_back();
                    if (frames_.empty()) {
                        stack_.pop_back();
                        // 如果是main的话，此后栈中就空了。但如果是其他线程，那么这里仍然有一些其他的本地变量，不过这也没什么问题。
                        return InterpreterResult::OK;
                    }
                    stack_.resize(last_frame.fp);
                    stack_.push_back(return_value);
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
                case Opcode::LoadEmptyString: {
                    push(LoxString::empty_string);
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
                    if (Flag::print_color) {
                        print_log(LoxValue::to_string(v) + "\n", Color::GREEN);
                    } else {
                        std::cout << LoxValue::to_string(v) << std::endl;
                    }
                    break;
                }
                case Opcode::Pop: {
                    pop();
                    break;
                }
                case Opcode::DefineGlobal: {
                    Value value = pop_and_get();
                    uint16_t str_id = read_operand_2();
                    std::lock_guard<std::mutex> lock(Runtime::globals_access_mutex);
                    Runtime::globals[str_id] = value;
                    break;
                }
                case Opcode::LoadGlobal: {
                    auto str_id = read_operand_2();

                    push(st_runtime.access_global(str_id));

                    // std::lock_guard<std::mutex> lock(Runtime::globals_access_mutex);
                    // auto found = Runtime::globals.find(str_id);
                    // if (found != Runtime::globals.end()) {
                    //     push(found->second);
                    //     break;
                    // }
                    // found = Runtime::builtin.find(str_id);
                    // if (found != Runtime::builtin.end()) {
                    //     push(found->second);
                    //     break;
                    // }
                    // throw LoxNameError(fmt::format("the variable: {} is not found", StringIntern::read_from_id(str_id)));
                    break;
                }
                case Opcode::SetGlobal: {
                    auto str_id = read_operand_2();
                    std::lock_guard<std::mutex> lock(Runtime::globals_access_mutex);
                    auto found = Runtime::globals.find(str_id);
                    if (found == Runtime::globals.end()) {
                        throw LoxNameError(fmt::format("the variable: {} is not found", StringIntern::read_from_id(str_id)));
                    } else {
                        Value v = stack_.back();
                        Runtime::globals[str_id] = v;
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
                    frame_at(index) = stack_.back();
                    break;
                }
                case Opcode::ClearN: {
                    auto amount = read_operand_1();
                    // a, b, c
                    escape_above(stack().size() - amount);
                    stack_.resize(stack_.size() - amount);
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
                    if (!LoxValue::to_bool(stack_.back())) {
                        pc() += distance;
                    }
                    break;
                }
                case Opcode::JumpIfTrue: {
                    auto distance = read_operand_2();
                    if (LoxValue::to_bool(stack_.back())) {
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
                    // call_value(arg_count);
                    call_closure_st(arg_count);
                    break;
                }
                case Opcode::MakeClosure: {
                    Value value = pop_and_get();
                    auto fun = LoxValue::to_reference_unsafe<LoxFunction>(value);
                    auto new_closure = Runtime::allocate_as<LoxClosure>(fun);
                    uint8_t num_captures = read_operand_1();
                    push(new_closure); // 先将closure入栈，否则捕获值在创建的过程中可能会被gc
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
                    Runtime::record_allocation(new_closure);
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
                    closure()->captureds.at(index)->value() = stack_.back();
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
                    for (uint8_t i = 0; i < count; i++) {
                        // a, b, c, x
                        Value v = stack_.at(stack_.size() - count + i);
                        s << LoxValue::to_string(v);
                    }
                    stack_.resize(stack_.size() - count);
                    LoxReference result = Runtime::allocate_as_ref<LoxString>(s.str());
                    push(result);
                    Runtime::record_allocation(result);
                    break;
                }
                case Opcode::MakeClass: {
                    auto str_id = read_operand_2();
                    auto name = StringIntern::read_from_id(str_id);
                    auto num_field = read_operand_1();
                    auto num_method = read_operand_1();
                    auto new_class = Runtime::allocate_as<LoxClass>(name, num_field);
                    for (size_t i = 0; i < num_method; i++) {
                        Value v = pop_and_get();
                        auto closure = LoxValue::to_reference_unsafe<LoxClosure>(v);
                        new_class->add_method(closure);
                    }
                    push(new_class);
                    Runtime::record_allocation(new_class);
                    break;
                }
                case Opcode::MethodBind: {
                    auto receiver = pop_and_get();
                    auto string_id = read_operand_2();
                    auto cl = method_lookup(receiver, string_id);
                    auto method = Runtime::allocate_as_ref<LoxMethod>(cl, LoxValue::to_reference_unsafe<LoxInstance>(receiver));
                    push(method);
                    Runtime::record_allocation(method);
                    break;
                }
                case Opcode::MethodInvoke: {
                    auto string_id = read_operand_2();
                    uint8_t arg_count = read_operand_1();
                    method_invoke(string_id, arg_count);
                    break;
                }
                case Opcode::LoadField: {
                    auto index = read_operand_1();
                    auto instance = LoxValue::to_reference_unsafe<LoxInstance>(frame_at(0));
                    push(instance->get_field(index));
                    break;
                }
                case Opcode::SetField: {
                    // [instance, to_set] => [to_set]
                    auto index = read_operand_1();
                    Value v = pop_and_get();
                    auto instance = LoxValue::to_reference_unsafe<LoxInstance>(frame_at(0));
                    instance->get_field(index) = v;
                    push(v);
                    break;
                }
                default:
                    IMPL_ERROR(fmt::format("unknown opcode inside the vm running. code num: {}",
                                                     static_cast<uint8_t>(instruction)));
            }
        }
    } catch (LoxError &error) {
        std::cerr << error.what() << std::endl;
        std::cerr << backtrace();
        return InterpreterResult::RuntimeError;
    }
}

std::string VM::backtrace() {
    std::stringstream result;
    for (auto it = frames_.rbegin(); it != frames_.rend(); it++) {
        auto call_frame = *it;
        auto line = call_frame.closure->function->get_chunk().get_line_num(call_frame.pc);
        result << fmt::format("{} at line {}\n", call_frame.closure->to_string(), line);
    }
    return result.str();
}

void VM::call_value(size_t arg_count) {
    if (frames_.size() == Configuration::frame_max) {
        throw LoxStackOverflowError("stack overflow");
    }
    size_t fp = stack_.size() - 1 - arg_count;
    Value callable = stack_.at(fp);

    if (std::holds_alternative<std::shared_ptr<LoxNativeFunction>>(callable)) {
        call_native(arg_count);
        return;
    }
    if (!std::holds_alternative<LoxReference>(callable)) {
        throw LoxTypeError(fmt::format("the value {} cannot not be called", LoxValue::to_string(callable)));
    }
    switch (std::get<LoxReference>(callable)->get_object_type()) {
        case LoxObjectType::Closure:
            call_closure(arg_count);
            break;
        case LoxObjectType::Method:
            call_method(arg_count);
            break;
        case LoxObjectType::Class:
            call_class(arg_count);
            break;
        default:
            throw LoxTypeError(fmt::format("the value {} cannot not be called", LoxValue::to_string(callable)));
    }
}

void VM::call_closure_st(size_t arg_count) {
    size_t fp = stack_.size() - 1 - arg_count;
    auto cl = LoxValue::to_reference_unsafe<LoxClosure>(stack_.at(fp));
    setup_frame(cl, fp);
}

void VM::call_closure(size_t arg_count) {
    size_t fp = stack_.size() - 1 - arg_count;
    auto cl = LoxValue::to_reference_unsafe<LoxClosure>(stack_.at(fp));
    if (cl->function->arity() != arg_count) {
        throw LoxArgError(fmt::format("the callable {} expect {} arguments, but got {}", LoxValue::to_string(cl),
                                      cl->function->arity(), arg_count));
    }
    setup_frame(cl, fp);
}

void VM::call_method(size_t arg_count) {
    size_t fp = stack_.size() - 1 - arg_count;
    auto method = LoxValue::to_reference_unsafe<LoxMethod>(stack_.at(fp));
    auto &cl = method->closure();
    if (cl->function->arity() != arg_count) {
        throw LoxArgError(fmt::format("the callable {} expect {} arguments, but got {}", LoxValue::to_string(cl),
                                      cl->function->arity(), arg_count));
    }
    setup_frame(cl, fp);
    stack_.at(fp) = method->receiver(); // 将帧底替换为receiver
}

void VM::call_class(size_t arg_count) {
    size_t fp = stack_.size() - 1 - arg_count;
    auto a_class = LoxValue::to_reference_unsafe<LoxClass>(stack_.at(fp));
    auto &constructor = a_class->constructor();
    if (constructor == nullptr) {
        // 无构造函数
        if (arg_count != 0) {
            throw LoxArgError(fmt::format("default constructor expect 0 argument, but got {}", arg_count));
        } else {
            // 默认构造
            LoxReference v = Runtime::allocate_as_ref<LoxInstance>(a_class, a_class->get_num_fields());
            stack_.resize(stack_.size() - 1 - arg_count);
            push(v);
            Runtime::record_allocation(v);
        }
    } else {
        if (arg_count != constructor->function->arity()) {
            throw LoxArgError(fmt::format("the constructor expect {} argument, but got {}",
                                          constructor->function->arity(),
                                          arg_count));
        } else {
            LoxReference v = Runtime::allocate_as_ref<LoxInstance>(a_class, a_class->get_num_fields());
            stack_.at(fp) = v;
            setup_frame(constructor, fp);
            Runtime::record_allocation(v);
        }
    }
}

void VM::call_native(size_t arg_count) {
    size_t fp = stack_.size() - 1 - arg_count;
    auto native = std::get<std::shared_ptr<LoxNativeFunction>>(stack().at(fp));
    if (arg_count != native->get_arity()) {
        throw LoxArgError(fmt::format("the native function {} expect {} arguments, but got {}", native->get_name(),
                                      native->get_arity(), arg_count));
    }
    auto impl = native->get_impl();
    impl(*this, fp);
}

void VM::print_log(const std::string &content, Color color) {
    if (log_stream.has_value()) {
        print_to(log_stream.value(), content, color);
        log_stream.value().flush();
    } else {
        print_to(std::cout, content, color);
        std::cout.flush();
    }
}

void VM::recur_call(size_t arg_count) {
    if (arg_count != closure()->function->arity()) {
        throw LoxArgError(fmt::format("the callable {} expect {} arguments, but got {}", LoxValue::to_string(closure()),
                                      closure()->function->arity(), arg_count));
    }
    for (uint8_t i = 0; i < arg_count; i++) {
        frame_at(1 + i) = stack_.at(stack_.size() - arg_count + i);
    }
    stack_.resize(curr_frame().fp + arg_count + 1);
    pc() = 0;
}

std::shared_ptr<LoxClosure> VM::method_lookup(const Value &receiver, OperandSize string_id) {
    if (std::holds_alternative<LoxReference>(receiver) == false) {
        throw LoxTypeError(fmt::format("the value {} is not an reference type and cannot bind to a method",
                                       LoxValue::to_string(receiver)));
    }
    auto ref = std::get<LoxReference>(receiver);
    if (!ref->is_of_type(LoxObjectType::Instance)) {
        throw LoxTypeError(fmt::format("the value {} is not an reference type and cannot bind to a method",
                                       LoxValue::to_string(receiver)));
    }
    auto instance = std::static_pointer_cast<LoxInstance>(ref);

    auto clo = instance->get_class()->resolve_method(string_id);

    return clo;
}

void VM::method_invoke(OperandSize string_id, uint8_t arg_count) {
    // receiver, arg1, arg2, arg3
    size_t fp = stack_.size() - arg_count - 1;
    Value receiver = stack_.at(fp);
    auto cl = method_lookup(receiver, string_id);
    if (cl->function->arity() != arg_count) {
        throw LoxArgError(fmt::format("the method {} expects {} arguments, but got {}", cl->to_string(), cl->function->arity(), arg_count));
    }
    setup_frame(cl, fp);
    stack_.at(fp) = receiver; // 修改栈底的接收者
}

void VM::start_blocking() {
    Runtime::wait_sync();
    this->active = false;
    if (Flag::show_heap) {
        // Runtime::print_log(fmt::format("@{} vm {} starts blocking\n", nanos_str(), vm_id), Color::BLUE);
    }
    Runtime::sync_flag.clear();
}

void VM::end_blocking() {
    while (Runtime::sync_flag.test_and_set()) {
    }
    if (Flag::show_heap) {
        // Runtime::print_log(fmt::format("@{} vm {} ends blocking\n", nanos_str(), vm_id), Color::BLUE);
    }
    this->active = true;
    Runtime::sync_flag.clear();
}
