#include "compiler.h"
#include "chunk.h"
#include "common.h"
#include "objects/loxfunction.h"
#include "objects/loxstring.h"
#include "scanner.h"
#include "value.h"
#include <algorithm>
#include <string>
#include <fmt/core.h>
#include <utility>

#include "runtime.h"

void Compiler::error_at(const Token &token, const std::string &message) {
    if (panic_mode) {
        return;
    }
    panic_mode = true;
    std::cerr << fmt::format("[line {}] Error", token.line);
    if (token.type == TokenType::END_OF_FILE) {
        std::cerr << " at end. ";
    } else if (token.type == TokenType::ERROR) {
    } else {
        std::cerr << fmt::format(" at {}. ", token.lexeme);
    }
    std::cerr << message << std::endl;
    has_error = true;
}

void Compiler::consume(TokenType type, const std::string &message) {
    if (next.type == type) {
        advance();
    } else {
        if (Flag::repl) {
            throw ConsumePending("pending" + message);
        } else {
            error_at(next, message);
        }
    }
}

void Compiler::advance() {
    curr = std::move(next);

    while (true) {
        next = scanner->scan_token();
        if (next.type != TokenType::ERROR) {
            break;
        } else {
            error_at(next, next.lexeme);
        }
    }
}

std::shared_ptr<LoxFunction> Compiler::compile(std::string &&source) {
    scanner = std::make_unique<Scanner>(std::move(source));
    scope = std::make_shared<Scope>(nullptr, FunctionType::Main);
    advance();
    while (!check(TokenType::END_OF_FILE)) {
        declaration();
    }
    emit_opcode(Opcode::LoadNil);
    emit_opcode(Opcode::Return);
    Runtime::record_allocation(scope->function_);
    if (has_error) {
        return nullptr;
    } else {
        if (Flag::disassembly) {
            disasm.set_chunk(&scope->function_->get_chunk());
            disasm.disassemble("<main>");
        }
        return scope->function_;
    }
}


Compiler::ParseFn Compiler::get_prefix(TokenType type) {
    switch (type) {
        case TokenType::LEFT_PAREN:
            return &Compiler::grouping_expr;
        case TokenType::MINUS:
        case TokenType::BANG:
            return &Compiler::unary_expr;
        case TokenType::INTEGER:
            return &Compiler::integer_expr;
        case TokenType::FLOAT:
            return &Compiler::float_expr;
        case TokenType::NIL:
        case TokenType::TRUE:
        case TokenType::FALSE:
            return &Compiler::literal_expr;
        case TokenType::STRING:
            return &Compiler::string_expr;
        case TokenType::FMT_STRING:
            return &Compiler::fmt_string_expr;
        case TokenType::IDENTIFIER:
            return &Compiler::variable_expr;
        case TokenType::THIS:
            return &Compiler::this_expr;
        default:
            return nullptr;
    }
}

auto Compiler::get_infix(TokenType type) -> Compiler::ParseFn {
    switch (type) {
        case TokenType::MINUS:
        case TokenType::PLUS:
        case TokenType::SLASH:
        case TokenType::STAR:
        case TokenType::STAR_STAR:
        case TokenType::EQUAL_EQUAL:
        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::BANG_EQUAL:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER_EQUAL:
            return &Compiler::binary_expr;
        case TokenType::AND:
            return &Compiler::and_expr;
        case TokenType::OR:
            return &Compiler::or_expr;
        case TokenType::LEFT_PAREN:
            return &Compiler::call_expr;
        case TokenType::DOT:
            return &Compiler::dot_expr;
        default:
            return nullptr;
    }
}

auto Compiler::get_precedence(TokenType type) -> Precedence {
    switch (type) {
        case TokenType::MINUS:
        case TokenType::PLUS:
            return Precedence::TERM;
        case TokenType::SLASH:
        case TokenType::STAR:
            return Precedence::FACTOR;
        case TokenType::STAR_STAR:
            return Precedence::POWER;
        case TokenType::EQUAL_EQUAL:
        case TokenType::BANG_EQUAL:
            return Precedence::EQUALITY;
        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER_EQUAL:
            return Precedence::COMPARISON;
        // case TokenType::NIL:
        // case TokenType::TRUE:
        // case TokenType::FALSE:
        // case TokenType::INTEGER:
        // case TokenType::FLOAT:
        // case TokenType::STRING:
        // case TokenType::FMT_STRING:
        // case TokenType::IDENTIFIER:
        //     return Precedence::PRIMARY;
        case TokenType::AND:
            return Precedence::AND;
        case TokenType::OR:
            return Precedence::OR;
        case TokenType::LEFT_PAREN:
        case TokenType::DOT:
            return Precedence::CALL;
        default:
            return Precedence::NONE;
    }
}

// ReSharper disable once CppDFAConstantParameter
void Compiler::compile_precedence_at_least(Precedence at_least) {
    /*
     * 取得next的prefix解析函数。
     * 如果为空，说明next不应该作为一个表达式的开头，因此属于编译错误。
     * 如果next不为空，说明next可以作为一个表达式的开头，则运行之。
     */
    advance();
    TokenType type = curr.type;
    ParseFn prefix = get_prefix(type);
    if (prefix == nullptr) {
        error_at(curr, fmt::format("expect an expression but get: {}", curr.lexeme));
        return;
    }
    bool can_assign = at_least <= Precedence::ASSIGNMENT;
    (this->*prefix)(can_assign); // 前缀运算符应该是没有用到can_assign这个属性的

    /*
     * 运行一个ParseFn之后，下一个要解析的token是next。我们判断next能否作为infix。
     * 如果可以，且其优先级大于等于at_least，我们应该继续解析。
     * 如果不可以，则到此为止。
     */
    // ReSharper disable once CppDFALoopConditionNotUpdated
    while (get_precedence(next.type) >= at_least) {
        advance();
        ParseFn infix = get_infix(curr.type); // 在经历上一行的advance()之后，这里的curr就是上面条件中的next。
        (this->*infix)(can_assign); // 这里的can_assign参数实际上并没有用，因为暂时没有任何infix真的用到了它
    }
}

void Compiler::integer_expr([[maybe_unused]] bool) {
    long integer = std::stol(curr.lexeme);
    auto opt_index = Chunk::to_immediate(integer);
    if (!opt_index) {
        emit_load_constant_flexible(integer);
    } else {
        emit_opcode(Opcode::LoadImmediate);
        emit_operand_1(opt_index.value());
    }
}

void Compiler::float_expr([[maybe_unused]] bool can_assign) {
    double decimal = std::stod(curr.lexeme);
    auto opt_index = Chunk::to_immediate(decimal);
    if (!opt_index) {
        emit_load_constant_flexible(decimal);
    } else {
        emit_opcode(Opcode::LoadImmediate);
        emit_operand_1(opt_index.value());
    }
}

void Compiler::compile_expression() {
    compile_precedence_at_least(Precedence::ASSIGNMENT);
}

void Compiler::grouping_expr([[maybe_unused]] bool can_assign) {
    compile_expression();
    consume(TokenType::RIGHT_PAREN, "expect ) after the expression");
}

void Compiler::binary_expr([[maybe_unused]] bool can_assign) {
    TokenType type = curr.type;
    Precedence precedence = get_precedence(type);
    compile_precedence_at_least(get_higher_precedence(precedence));
    switch (type) {
        case TokenType::MINUS:
            emit_opcode(Opcode::Subtract);
            break;
        case TokenType::PLUS:
            emit_opcode(Opcode::Add);
            break;
        case TokenType::SLASH:
            emit_opcode(Opcode::Divide);
            break;
        case TokenType::STAR:
            emit_opcode(Opcode::Multipy);
            break;
        case TokenType::STAR_STAR:
            emit_opcode(Opcode::Power);
            break;
        case TokenType::BANG:
            emit_opcode(Opcode::Not);
            break;
        case TokenType::GREATER:
            emit_opcode(Opcode::Greater);
            break;
        case TokenType::LESS:
            emit_opcode(Opcode::Less);
            break;
        case TokenType::EQUAL_EQUAL:
            emit_opcode(Opcode::Equal);
            break;
        case TokenType::BANG_EQUAL:
            emit_opcode(Opcode::Equal);
            emit_opcode(Opcode::Not);
            break;
        case TokenType::GREATER_EQUAL:
            emit_opcode(Opcode::Less);
            emit_opcode(Opcode::Not);
            break;
        case TokenType::LESS_EQUAL:
            emit_opcode(Opcode::Greater);
            emit_opcode(Opcode::Not);
            break;
        default:
            return;
    }
}

void Compiler::dot_expr(bool can_assign) {
    // this.num = 10; dog.run(); var a = dog.eat;
    // a.b().c()

    consume(TokenType::IDENTIFIER, "expect an identifier after '.'");
    auto key = current_chunk().add_identifier(curr.get_lexeme());
    auto cache_index = current_chunk().add_method_cache();
    if (match(TokenType::LEFT_PAREN)) {
        auto arg_count = argument_list();
        emit_opcode(Opcode::MethodInvoke);
        emit_operand_2(key);
        emit_operand_1(cache_index);
        emit_operand_1(arg_count);
    } else {
        emit_opcode(Opcode::MethodBind);
        emit_operand_2(key);
        emit_operand_1(cache_index);
    }
}

void Compiler::and_expr([[maybe_unused]] bool can_assign) {
    // a and b and c
    auto short_circuit = emit_jump(Opcode::JumpIfFalse);
    emit_opcode(Opcode::Pop);

    compile_precedence_at_least(Precedence::AND);

    patch_jump(short_circuit);
}

void Compiler::or_expr([[maybe_unused]] bool can_assign) {
    // a or b or c
    auto short_circuit = emit_jump(Opcode::JumpIfFalse);
    auto end = emit_jump(Opcode::Jump);

    patch_jump(short_circuit);

    emit_opcode(Opcode::Pop);
    compile_precedence_at_least(Precedence::OR);
    patch_jump(end);
}

void Compiler::literal_expr([[maybe_unused]] bool can_assign) {
    TokenType type = curr.type;
    switch (type) {
        case TokenType::NIL:
            emit_opcode(Opcode::LoadNil);
            break;
        case TokenType::TRUE:
            emit_opcode(Opcode::LoadTrue);
            break;
        case TokenType::FALSE:
            emit_opcode(Opcode::LoadFalse);
            break;
        default:
            DEBUG_ASSERT(false, "this is should be unreachable");
    }
}

void Compiler::string_expr([[maybe_unused]] bool can_assign) {
    LoxReference value = Runtime::allocate_as_ref<LoxString>(curr.lexeme.substr(1, curr.lexeme.size() - 2));
    Runtime::record_allocation(value);
    emit_load_constant_flexible(std::move(value));
}

void Compiler::fmt_string_expr([[maybe_unused]] bool can_assign) {
    auto s = curr.lexeme.substr(1, curr.lexeme.size() - 2); // 去除头尾的引号
    int count = 0;
    auto ranges = Scanner::split(s);
    if (!ranges) {
        throw FmtStringUnbalancedError("the format string is not balanced");
    }
    auto saved_scanner = std::move(scanner);
    Token saved_next = next; // next要被保存。因为切换scanner进行解析的过程中会被修改
    int curr_line = curr.line;
    for (auto [caught, left, right]: ranges.value()) {
        if (!caught) {
            // 普通字符串
            LoxReference str = Runtime::allocate_as_ref<LoxString>(s.substr(left, right - left + 1));
            Runtime::record_allocation(str);
            emit_load_constant_flexible(str);
            count++;
        } else {
            // 内嵌表达式。这里去除掉了头尾的{}
            std::string inner_expr = s.substr(left + 1, right - left + 1 - 2);
            scanner = std::make_unique<Scanner>(std::move(inner_expr), curr_line);
            next = Token{}; // 重设next。防止使用上一个scanner遗留下的next
            advance(); // 使next是下一个要解析的token（如果不这么做，next最初是空的）
            if (next.type == TokenType::END_OF_FILE) {
                continue;
            }
            compile_expression();
            count++;
        }
    }
    scanner = std::move(saved_scanner);
    next = saved_next;
    if (count > 1) {
        emit_opcode(Opcode::StringConcat);
        if (!within<uint8_t>(count)) {
            throw Uint8OperandOverflowError("too many expressions for a format string");
        }
        emit_operand_1(count);
    }
}

void Compiler::call_expr([[maybe_unused]] bool can_assign) {
    uint8_t arg_count = argument_list();
    emit_opcode(Opcode::Call);
    emit_operand_1(arg_count);
}

uint8_t Compiler::argument_list() {
    size_t arg_count = 0;
    if (match(TokenType::RIGHT_PAREN)) {
        return arg_count;
    }
    do {
        compile_expression();
        arg_count++;
    } while (match(TokenType::COMMA));
    consume(TokenType::RIGHT_PAREN);
    if (within<uint8_t>(arg_count) == false) {
        throw Uint8OperandOverflowError("too many arguments");
    }
    return arg_count;
}

void Compiler::unary_expr([[maybe_unused]] bool can_assign) {
    TokenType type = curr.type;
    compile_precedence_at_least(Precedence::UNARY);
    if (type == TokenType::MINUS) {
        emit_opcode(Opcode::Negate);
    } else if (type == TokenType::BANG) {
        emit_opcode(Opcode::Not);
    }
}

void Compiler::variable_expr(bool can_assign) {
    auto found = scope->resolve_local(curr);

    if (found.has_value()) {
        // 是本地变量
        if (match(TokenType::EQUAL)) {
            if (can_assign) {
                compile_precedence_at_least(Precedence::ASSIGNMENT);
                emit_opcode(Opcode::SetLocal);
            } else {
                error_at(curr, fmt::format("invalid assignment target"));
            }
        } else {
            emit_opcode(Opcode::LoadLocal);
        }
        emit_operand_1(found.value());
        return;
    }

    if (scope->function_type_ == FunctionType::Method) {
        // 如果在方法内部，则还可能是对象字段。
        auto field_found = class_scope->resolve_field(curr.get_lexeme());
        if (field_found) {
            if (match(TokenType::EQUAL)) {
                if (can_assign) {
                    // emit_opcode(Opcode::LoadLocal);
                    // emit_operand_1(0);
                    compile_precedence_at_least(Precedence::ASSIGNMENT);
                    emit_opcode(Opcode::SetField);
                    emit_operand_1(field_found.value());
                } else {
                    error_at(curr, fmt::format("invalid assignment target"));
                }
            } else {
                // emit_opcode(Opcode::LoadLocal);
                // emit_operand_1(0);
                emit_opcode(Opcode::LoadField);
                emit_operand_1(field_found.value());
            }
            return;
        }
    }

    // todo: this.method() 如何处理?

    // 查找upvalues
    found = scope->resolve_upvalue(curr);

    if (found.has_value()) {
        // 在upvalue中找到了。
        if (match(TokenType::EQUAL)) {
            if (can_assign) {
                compile_precedence_at_least(Precedence::ASSIGNMENT);
                emit_opcode(Opcode::SetCaptured);
            } else {
                error_at(curr, fmt::format("invalid assignment target"));
            }
        } else {
            emit_opcode(Opcode::LoadCaptured);
        }
        emit_operand_1(found.value());
        return;
    }

    // 都没找到，则认为是全局变量
    OperandSize key = current_chunk().add_identifier(curr.lexeme);
    if (match(TokenType::EQUAL)) {
        if (can_assign) {
            compile_precedence_at_least(Precedence::ASSIGNMENT);
            emit_opcode(Opcode::SetGlobal);
        } else {
            error_at(curr, fmt::format("invalid assignment target"));
        }
    } else {
        emit_opcode(Opcode::LoadGlobal);
    }
    emit_operand_2(key);
}

void Compiler::this_expr(bool can_assign) {
    // this.num = 10;
    // this.run();
    // var age = this.num = 4;
    // var age = this.num;

    if (scope->function_type_ != FunctionType::Method) {
        error_at(curr, "'this' can only be used inside of a class method");
        return;
    }
    if (match(TokenType::DOT)) {
        // this.identifier
        consume(TokenType::IDENTIFIER, "expect an identifier after '.'");
        auto field_found = class_scope->resolve_field(curr.lexeme);
        if (field_found) {
            // 找到了对应的字段

            if (match(TokenType::EQUAL)) {
                // 写入
                if (can_assign) {
                    // 允许赋值
                    // emit_opcode(Opcode::LoadLocal);
                    // emit_operand_1(0);
                    compile_precedence_at_least(get_higher_precedence(Precedence::ASSIGNMENT));
                    emit_opcode(Opcode::SetField);
                    emit_operand_1(field_found.value());
                } else {
                    // 不允许赋值
                    error_at(curr, "invalid assignment target");
                }
            } else {
                // 读取
                // emit_opcode(Opcode::LoadLocal);
                // emit_operand_1(0);
                emit_opcode(Opcode::LoadField);
                emit_operand_1(field_found.value());
            }
        } else {
            emit_opcode(Opcode::LoadLocal);
            emit_operand_1(0);

            // 没有找到字段，则判断是方法
            auto key = current_chunk().add_identifier(curr.get_lexeme());
            auto cache_index = current_chunk().add_method_cache();
            if (match(TokenType::LEFT_PAREN)) {
                uint8_t arg_count = argument_list();
                emit_opcode(Opcode::MethodInvoke);
                emit_operand_2(key);
                emit_operand_1(cache_index);
                emit_operand_1(arg_count);
            } else {
                emit_opcode(Opcode::MethodBind);
                emit_operand_2(key);
                emit_operand_1(cache_index);
            }
        }
    } else {
        // 单独的this
        emit_opcode(Opcode::LoadLocal);
        emit_operand_1(0);
    }
}

void Compiler::emit_load_constant_flexible(Value &&value) {
    try {
        OperandSize index = current_chunk().add_constant(std::move(value));
        if (within<uint8_t>(index)) {
            emit_opcode(Opcode::LoadConstant);
        } else {
            emit_opcode(Opcode::LoadConstant2);
        }
        emit_operand_flexible(index);
    } catch (ConstantPoolOverflowError &err) {
        error_at(curr, err.what());
    }
}

void Compiler::synchronize() {
    panic_mode = false;
    if (curr.type == TokenType::END_OF_FILE) {
        return;
    }
    while (true) {
        if (curr.type == TokenType::SEMICOLON) {
            return;
        }
        switch (next.type) {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::WHILE:
            case TokenType::IF:
            case TokenType::PRINT:
            case TokenType::RETURN:
            case TokenType::END_OF_FILE:
                return;
            default:
                ; // 不做事情，只是为了消除编译警告
        }
        advance();
    }
}

void Compiler::print_statement() {
    compile_expression();
    emit_opcode(Opcode::Print);
    consume();
}

void Compiler::return_statement() {
    if (match(TokenType::SEMICOLON)) {
        emit_opcode(Opcode::LoadNil);
        emit_opcode(Opcode::Return);
    } else {
        compile_expression();
        emit_opcode(Opcode::Return);
        consume();
    }
}

void Compiler::recur_statement() {
    consume(TokenType::LEFT_PAREN, "expect a '(' after recur");

    auto arg_count = argument_list();
    consume();
    emit_opcode(Opcode::Recur);
    emit_operand_1(arg_count);
}


void Compiler::if_statement() {
    /**
     * condition
     * jump if pop false -> else
     * then:
     *      ...
     *     jump -> end
     * else:
     *      ...
     * end:
     */

    // condition
    compile_expression();

    // jump if pop false -> else
    auto to_else = emit_jump(Opcode::JumpIfPopFalse);

    // then_statement
    statement();

    if (match(TokenType::ELSE)) {
        // jump -> end
        auto to_end = emit_jump(Opcode::Jump);

        // else_statement
        patch_jump(to_else);
        statement();

        // end
        patch_jump(to_end);
    } else {
        // end/else
        patch_jump(to_else);
    }
}

void Compiler::while_statement() {
    size_t condition_label = current_chunk().code_size();

    auto old_continue_point = save_continue_point();
    compile_expression();

    auto old_breakpoint = save_breakpoint();
    auto to_end = emit_jump(Opcode::JumpIfPopFalse);

    statement();
    loop_back(condition_label);
    patch_jump(to_end);

    restore_breakpoint(old_breakpoint);
    restore_continue_point(old_continue_point);
}

void Compiler::break_statement() {
    if (!breakpoint) {
        error_at(curr, "cannot use break outside of a loop");
        return;
    }
    auto [dest, locals_size] = breakpoint.value();
    emit_opcode(Opcode::PopN);
    emit_operand_1(scope->locals_size() - locals_size);
    emit_opcode(Opcode::LoadFalse);
    loop_back(dest);
    consume();
}

void Compiler::continue_statement() {
    if (!continue_point) {
        error_at(curr, "cannot use continue outside of a loop");
        return;
    }
    auto [dest, locals_size] = continue_point.value();
    emit_opcode(Opcode::PopN);
    emit_operand_1(scope->locals_size() - locals_size);
    loop_back(dest);
    consume();
}


void Compiler::block_statement() {
    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::END_OF_FILE)) {
        declaration();
    }
    consume(TokenType::RIGHT_BRACE, "expect a '}' to end the block");
}

OperandSize Compiler::resolve_global_identifier() {
    consume(TokenType::IDENTIFIER, "expect an identifier here");
    return current_chunk().add_identifier(curr.lexeme);
}

void Compiler::fun_statement() {
    if (scope->is_global_scope()) {
        OperandSize key = resolve_global_identifier();

        auto [fun, fun_scope] = parse_function(FunctionType::Function);

        emit_make_closure(fun, fun_scope);

        emit_opcode(Opcode::DefineGlobal);
        emit_operand_2(key);
    } else {
        consume(TokenType::IDENTIFIER, "expect an identifier for the function");

        scope->add_local(curr);
        scope->initialize();
        auto [fun, fun_scope] = parse_function(FunctionType::Function);

        emit_make_closure(fun, fun_scope);
    }
}

void Compiler::class_statement() {
    if (scope->is_global_scope()) {
        OperandSize key = resolve_global_identifier();
        parse_class();
        emit_opcode(Opcode::DefineGlobal);
        emit_operand_2(key);
    } else {
        consume(TokenType::IDENTIFIER, "expect an identifier as the class name");
        scope->add_local(curr);
        scope->initialize();
        parse_class();
    }
}


void Compiler::var_statement() {
    try {
        if (scope->is_global_scope()) {
            // 全局变量
            OperandSize key = resolve_global_identifier();
            if (match(TokenType::EQUAL)) {
                compile_expression();
            } else {
                emit_opcode(Opcode::LoadNil);
            }
            consume();
            emit_opcode(Opcode::DefineGlobal);
            emit_operand_2(key);
        } else {
            // 本地变量
            consume(TokenType::IDENTIFIER, "expect an identifier here");
            scope->add_local(curr);

            if (match(TokenType::EQUAL)) {
                compile_expression();
            } else {
                emit_opcode(Opcode::LoadNil);
            }
            scope->initialize(); // 本地变量不需要专门的DefineLocal指令
            consume();
        }
    } catch (CompilerError &err) {
        error_at(curr, err.what());
    }
}

void Compiler::field_statement(bool is_static) {
    consume(TokenType::IDENTIFIER, "expect the field name here");
    class_scope->add_field(curr.get_lexeme());
    if (check(TokenType::EQUAL)) {
        error_at(next, "a field can only be initialized in the init method");
        return;
    }
    consume();
}

void Compiler::method_statement(bool is_static) {
    consume(TokenType::IDENTIFIER, "expect the method name here");
    auto [fun, fun_scope] = parse_function(FunctionType::Method);
    emit_make_closure(fun, fun_scope);
}

void Compiler::parse_class() {
    std::string class_name = curr.get_lexeme();
    auto key = current_chunk().add_identifier(class_name);
    consume(TokenType::LEFT_BRACE, "expect a '{' after the class name");
    class_scope = ClassScope{};
    bool field_finished = false; // 方法都必须写在所有的field后面
    while (match(TokenType::RIGHT_BRACE) == false) {
        if (match(TokenType::Static)) {
            if (match(TokenType::Field)) {
                if (field_finished) {
                    error_at(curr, "cannot declare fields after any methods");
                }
                field_statement(true);
            } else if (match(TokenType::Method)) {
                field_finished = true;
                method_statement(true);
                class_scope->incre_method_count();
            } else {
                error_at(next, "expect field or method definition after static");
                return;
            }
        } else {
            if (match(TokenType::Field)) {
                if (field_finished) {
                    error_at(curr, "cannot declare fields after any methods");
                }
                field_statement(true);
            } else if (match(TokenType::Method)) {
                field_finished = true;
                method_statement(true);
                class_scope->incre_method_count();
            } else {
                error_at(next, "expect field or method definition inside a class");
                return;
            }
        }
    }
    // consume(TokenType::RIGHT_BRACE, "expect a '}' to end the class definition");
    emit_opcode(Opcode::MakeClass);
    emit_operand_2(key);
    emit_operand_1(class_scope->member_fields.size());
    emit_operand_1(class_scope->method_count);

    class_scope = std::nullopt;
}

void Compiler::expression_statement() {
    compile_expression();
    consume();
    emit_opcode(Opcode::Pop);
}

// ReSharper disable once CppDFAConstantParameter
std::pair<std::shared_ptr<LoxFunction>, std::shared_ptr<Scope> > Compiler::parse_function(FunctionType type) {
    std::string fun_name = curr.get_lexeme();

    auto new_scope = std::make_shared<Scope>(scope, type);

    scope = new_scope;

    consume(TokenType::LEFT_PAREN, "expect a '(' after the function name");
    scope->step_into();

    if (match(TokenType::RIGHT_PAREN)) {
        goto after_param_list;
    }

    do {
        consume(TokenType::IDENTIFIER, "expect a parameter here");
        scope->add_local(curr);
        scope->function_->incre_arity();
        scope->initialize();
    } while (match(TokenType::COMMA));

    consume(TokenType::RIGHT_PAREN, "expect a ')' to end the parameter list");

after_param_list:
    consume(TokenType::LEFT_BRACE, "expect a '{' to start the function body");
    block_statement();

    // 这里不需要退出层级的操作，因为函数调用后，整个栈帧都会被废弃，栈vector会resize至前一个栈帧的尺寸
    // 所有本栈帧的本地变量自然也就被销毁了

    if (fun_name == "init" and type == FunctionType::Method) {
        emit_opcode(Opcode::LoadLocal); // 构造函数返回this
        emit_operand_1(0);
    } else {
        emit_opcode(Opcode::LoadNil); // 默认返回值为nil
    }
    emit_opcode(Opcode::Return);


    scope->function_->set_name(fun_name);

    Runtime::record_allocation(scope->function_);

    if (!has_error && Flag::disassembly) {
        disasm.set_chunk(&scope->function_->get_chunk());
        disasm.disassemble(fun_name);
    }

    auto function_value = scope->function_;
    scope = scope->outer_;

    return {function_value, new_scope};
}

void Compiler::statement() {
    if (match(TokenType::PRINT)) {
        print_statement();
    } else if (match(TokenType::LEFT_BRACE)) {
        auto old_size = scope->step_into();
        block_statement();
        auto amount_to_pop = scope->step_out(old_size);
        emit_opcode(Opcode::PopN);
        emit_operand_1(amount_to_pop);
    } else if (match(TokenType::IF)) {
        if_statement();
    } else if (match(TokenType::WHILE)) {
        while_statement();
    } else if (match(TokenType::BREAK)) {
        break_statement();
    } else if (match(TokenType::CONTINUE)) {
        continue_statement();
    } else if (match(TokenType::RETURN)) {
        return_statement();
    } else if (match(TokenType::RECUR)) {
        recur_statement();
    } else {
        expression_statement();
    }
}

void Compiler::declaration() {
    if (panic_mode) {
        synchronize();
    }
    try {
        if (match(TokenType::VAR)) {
            var_statement();
        } else if (match(TokenType::FUN)) {
            fun_statement();
        } else if (match(TokenType::CLASS)) {
            class_statement();
        } else {
            statement();
        }
    } catch (CompilerError &error) {
        error_at(curr, error.what());
    }
}

size_t Compiler::emit_jump(Opcode jump_instruction) {
    emit_opcode(jump_instruction);
    emit_operand_2(0);
    return current_chunk().code_size() - 2;
}

void Compiler::loop_back(size_t destination) {
    /**
     * destination <-- dest
     * i
     * i
     * loop_back   <-- 计算距离的时候，jumpback指令还没有生成，因此是在这里计算的
     * op1    
     * op2
     * next instruction  <-- 实际运行的时候，jump指令会读取两个操作符，因此pc会在这里
     * .
     * current
     */
    DEBUG_ASSERT(current_chunk().code_size() >= destination, "loop back is jumping forward!");
    size_t distance = current_chunk().code_size() - destination + 3;

    if (!within<uint16_t>(distance)) {
        throw JumpDistanceOverflowError("the distance to jump is too much to be encoded as an uint16");
    }
    emit_opcode(Opcode::JumpBack);
    emit_operand_2(distance);
}

void Compiler::patch_jump(size_t from_label) {
    /**
     * jump
     * op1    <-- 这里是from_label
     * op2
     * next instruction  <-- 实际运行的时候，jump指令会读取两个操作符，因此pc会在这里
     * .
     * current
     */

    // from_label的位置是jump指令的第一个操作数。但实际在执行jump语句的时候，pc的位置是jump语句的第二个操作数之后，因此跳转距离-2
    size_t distance = current_chunk().code_size() - from_label - 2;
    if (!within<uint16_t>(distance)) {
        throw JumpDistanceOverflowError("the distance to jump is too much to be encoded as an uint16");
    }
    auto [high, low] = u16_to_u8(distance);
    current_chunk().code_at(from_label) = low;
    current_chunk().code_at(from_label + 1) = high;
}


Compiler::BackPoint Compiler::save_continue_point() {
    auto old = continue_point;
    continue_point = {current_chunk().code_size(), scope->locals_size()};
    return old;
}

void Compiler::restore_continue_point(BackPoint old) {
    continue_point = old;
}

Compiler::BackPoint Compiler::save_breakpoint() {
    auto old = breakpoint;
    breakpoint = {current_chunk().code_size(), scope->locals_size()};
    return old;
}

void Compiler::restore_breakpoint(Compiler::BackPoint old) {
    breakpoint = old;
}
