#include "compiler.h"
#include "chunk.h"
#include "common.h"
#include "objects/loxstring.h"
#include "scanner.h"
#include <cstddef>
#include <string>
#include <fmt/core.h>

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

void Compiler::consume(TokenType type = TokenType::SEMICOLON,
                       const std::string &message = "expect a ';' to end the statement") {
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

std::shared_ptr<Chunk> Compiler::compile(std::string &&source) {
    scanner = std::make_unique<Scanner>(std::move(source));
    chunk = std::make_shared<Chunk>();
    scope = std::make_shared<Scope>();
    advance();
    //    compile_expression();
    while (!check(TokenType::END_OF_FILE)) {
        declaration();
    }
    end_compiler();
    if (has_error) {
        return nullptr;
    } else {
        return chunk;
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
        case TokenType::IDENTIFIER:
            return &Compiler::variable_expr;
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
        case TokenType::EQUAL_EQUAL:
        case TokenType::BANG_EQUAL:
            return Precedence::EQUALITY;
        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER_EQUAL:
            return Precedence::COMPARISON;
        case TokenType::NIL:
        case TokenType::TRUE:
        case TokenType::FALSE:
        case TokenType::INTEGER:
        case TokenType::FLOAT:
        case TokenType::STRING:
        case TokenType::IDENTIFIER:
            return Precedence::PRIMARY;
        case TokenType::AND:
            return Precedence::AND;
        case TokenType::OR:
            return Precedence::OR;
        default:
            return Precedence::NONE;
    }
}

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
    while (get_precedence(next.type) >= at_least) {
        // 这里clion会给出警告，可以无视它。
        advance();
        ParseFn infix = get_infix(curr.type); // 在经历上一行的advance()之后，这里的curr就是上面条件中的next。
        (this->*infix)(can_assign); // 这里的can_assign参数实际上并没有用，因为暂时没有任何infix真的用到了它
    }
}

void Compiler::integer_expr([[maybe_unused]] bool) {
    long integer = std::stol(curr.lexeme);
    uint8_t index = Chunk::to_immediate(integer);
    if (index == 255) {
        emit_load_constant(integer);
    } else {
        emit_opcode(OpCode::LoadImmediate);
        emit_operand(index);
    }
}

void Compiler::float_expr([[maybe_unused]] bool can_assign) {
    double decimal = std::stod(curr.lexeme);
    uint8_t index = Chunk::to_immediate(decimal);
    if (index == 255) {
        emit_load_constant(decimal);
    } else {
        emit_opcode(OpCode::LoadImmediate);
        emit_operand(index);
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
            emit_opcode(OpCode::Subtract);
            break;
        case TokenType::PLUS:
            emit_opcode(OpCode::Add);
            break;
        case TokenType::SLASH:
            emit_opcode(OpCode::Divide);
            break;
        case TokenType::STAR:
            emit_opcode(OpCode::Multipy);
            break;
        case TokenType::BANG:
            emit_opcode(OpCode::Not);
            break;
        case TokenType::GREATER:
            emit_opcode(OpCode::Greater);
            break;
        case TokenType::LESS:
            emit_opcode(OpCode::Less);
            break;
        case TokenType::EQUAL_EQUAL:
            emit_opcode(OpCode::Equal);
            break;
        case TokenType::BANG_EQUAL:
            emit_opcode(OpCode::Equal);
            emit_opcode(OpCode::Not);
            break;
        case TokenType::GREATER_EQUAL:
            emit_opcode(OpCode::Less);
            emit_opcode(OpCode::Not);
            break;
        case TokenType::LESS_EQUAL:
            emit_opcode(OpCode::Greater);
            emit_opcode(OpCode::Not);
            break;
        default:
            return;
    }
}

void Compiler::and_expr([[maybe_unused]] bool can_assign) {
    // a and b and c
    auto short_circuit = emit_jump(OpCode::JumpIfFalse);
    emit_opcode(OpCode::Pop);

    compile_precedence_at_least(Precedence::AND);

    patch_jump(short_circuit);
}

void Compiler::or_expr([[maybe_unused]] bool can_assign) {
    // a or b or c
    auto short_circuit = emit_jump(OpCode::JumpIfFalse);
    auto end = emit_jump(OpCode::Jump);

    patch_jump(short_circuit);

    emit_opcode(OpCode::Pop);
    compile_precedence_at_least(Precedence::OR);
    patch_jump(end);
}

void Compiler::literal_expr([[maybe_unused]] bool can_assign) {
    TokenType type = curr.type;
    switch (type) {
        case TokenType::NIL:
            emit_opcode(OpCode::LoadNil);
            break;
        case TokenType::TRUE:
            emit_opcode(OpCode::LoadTrue);
            break;
        case TokenType::FALSE:
            emit_opcode(OpCode::LoadFalse);
            break;
        default:
            DEBUG_ASSERT(false, "this is should be unreachable");
    }
}

void Compiler::string_expr([[maybe_unused]] bool can_assign) {
    Value value = LoxObject::allocate<LoxString>(curr.lexeme.substr(1, curr.lexeme.size() - 2));
    emit_load_constant(std::move(value));
}

void Compiler::unary_expr([[maybe_unused]] bool can_assign) {
    TokenType type = curr.type;
    compile_precedence_at_least(Precedence::UNARY);
    if (type == TokenType::MINUS) {
        emit_opcode(OpCode::Negate);
    } else if (type == TokenType::BANG) {
        emit_opcode(OpCode::Not);
    }
}

void Compiler::variable_expr(bool can_assign) {
    int local_index = scope->resolve_local(curr);

    if (local_index != -1) {
        // 是本地变量
        if (match(TokenType::EQUAL)) {
            if (can_assign) {
                compile_precedence_at_least(Precedence::ASSIGNMENT);
                emit_opcode(OpCode::SetLocal);
            } else {
                error_at(curr, fmt::format("invalid assignment target"));
            }
        } else {
            emit_opcode(OpCode::LoadLocal);
        }
        emit_operand(local_index);
    } else {
        // 本地没有找到，则认为是全局变量
        OperandSize key = current_chunk()->add_identifier(curr.lexeme);
        if (match(TokenType::EQUAL)) {
            if (can_assign) {
                compile_precedence_at_least(Precedence::ASSIGNMENT); // todo: 原书中是expression()
                emit_opcode(OpCode::SetGlobal);
            } else {
                error_at(curr, fmt::format("invalid assignment target"));
            }
        } else {
            emit_opcode(OpCode::LoadGlobal);
        }
        emit_operand_2(key);
    }
}

void Compiler::emit_load_constant(Value &&value) {
    try {
        OperandSize index = current_chunk()->add_constant(std::move(value));
        if (within<uint8_t>(index)) {
            emit_opcode(OpCode::LoadConstant);
        } else if (within<uint16_t>(index)) {
            emit_opcode(OpCode::LoadConstant2);
        } else {
            implementation_error("an overflowed index is returned without throwing!");
        }
        emit_operand(index);
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
    emit_opcode(OpCode::Print);
    consume();
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
    auto to_else = emit_jump(OpCode::JumpIfPopFalse);

    // then_statement
    statement();

    if (match(TokenType::ELSE)) {
        // jump -> end
        auto to_end = emit_jump(OpCode::Jump);

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
    size_t condition_label = current_chunk()->code_size();

    auto old_continue_point = save_continue_point();
    compile_expression();

    auto old_breakpoint = save_breakpoint();
    auto to_end = emit_jump(OpCode::JumpIfPopFalse);

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
    emit_opcode(OpCode::PopN);
    emit_operand(scope->locals_size() - locals_size);
    emit_opcode(OpCode::LoadFalse);
    loop_back(dest);
    consume();
}

void Compiler::continue_statement() {
    if (!continue_point) {
        error_at(curr, "cannot use continue outside of a loop");
        return;
    }
    auto [dest, locals_size] = continue_point.value();
    emit_opcode(OpCode::PopN);
    emit_operand(scope->locals_size() - locals_size);
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
    return current_chunk()->add_identifier(curr.lexeme);
}

void Compiler::var_statement() {
    try {
        if (scope->is_global_scope()) {
            // 全局变量
            OperandSize key = resolve_global_identifier();
            if (match(TokenType::EQUAL)) {
                compile_expression();
            } else {
                emit_opcode(OpCode::LoadNil);
            }
            consume();
            emit_opcode(OpCode::DefineGlobal);
            emit_operand_2(key);
        } else {
            // 本地变量
            consume(TokenType::IDENTIFIER, "expect an identifier here");
            scope->add_local(curr);

            if (match(TokenType::EQUAL)) {
                compile_expression();
            } else {
                emit_opcode(OpCode::LoadNil);
            }
            scope->initialize(); // 本地变量不需要专门的DefineLocal指令
            consume();
        }
    } catch (InterpreterError &err) {
        error_at(curr, err.what());
    }
}

void Compiler::expression_statement() {
    compile_expression();
    consume();
    emit_opcode(OpCode::Pop);
}

void Compiler::statement() {
    if (match(TokenType::PRINT)) {
        print_statement();
    } else if (match(TokenType::LEFT_BRACE)) {
        auto old_size = scope->step_into();
        block_statement();
        auto amount_to_pop = scope->step_out(old_size);
        emit_opcode(OpCode::PopN);
        emit_operand(amount_to_pop);
    } else if (match(TokenType::IF)) {
        if_statement();
    } else if (match(TokenType::WHILE)) {
        while_statement();
    } else if (match(TokenType::BREAK)) {
        break_statement();
    } else if (match(TokenType::CONTINUE)) {
        continue_statement();
    } else {
        expression_statement();
    }
}

void Compiler::declaration() {
    if (panic_mode) {
        synchronize();
    }
    if (match(TokenType::VAR)) {
        var_statement();
    } else {
        statement();
    }
}

size_t Compiler::emit_jump(OpCode jump_instruction) {
    emit_opcode(jump_instruction);
    emit_operand_2(0);
    return current_chunk()->code_size() - 2;
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
    DEBUG_ASSERT(current_chunk()->code_size() >= destination, "loop back is jumping forward!");
    size_t distance = current_chunk()->code_size() - destination + 3;

    if (!within<uint16_t>(distance)) {
        throw JumpDistanceOverflowError("the distance to jump is too much to be encoded as an uint16");
    }
    emit_opcode(OpCode::JumpBack);
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
    size_t distance = current_chunk()->code_size() - from_label - 2;
    if (!within<uint16_t>(distance)) {
        throw JumpDistanceOverflowError("the distance to jump is too much to be encoded as an uint16");
    }
    auto [high, low] = u16_to_u8(distance);
    current_chunk()->code_at(from_label) = low;
    current_chunk()->code_at(from_label + 1) = high;
}


Compiler::BackPoint Compiler::save_continue_point() {
    auto old = continue_point;
    continue_point = {current_chunk()->code_size(), scope->locals_size()};
    return old;
}

void Compiler::restore_continue_point(BackPoint old) {
    continue_point = old;
}

Compiler::BackPoint Compiler::save_breakpoint() {
    auto old = breakpoint;
    breakpoint = {current_chunk()->code_size(), scope->locals_size()};
    return old;
}

void Compiler::restore_breakpoint(Compiler::BackPoint old) {
    breakpoint = old;
}
