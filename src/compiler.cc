#include "compiler.h"
#include "objects/loxstring.h"
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
        std::cerr << fmt::format(" at {}", token.lexeme);
    }
    std::cerr << message << std::endl;
    has_error = true;
}

void Compiler::consume(TokenType type = TokenType::SEMICOLON, const std::string &message = "expect a ';' to end the statement") {
    if (next.type == type) {
        advance();
    } else {
        error_at(next, message);
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
    (this->*prefix)();

    /*
     * 运行一个ParseFn之后，下一个要解析的token是next。我们判断next能否作为infix。
     * 如果可以，且其优先级大于等于at_least，我们应该继续解析。
     * 如果不可以，则到此为止。
     */
    while (get_precedence(next.type) >= at_least) { // 这里clion会给出警告，可以无视它。
        advance();
        ParseFn infix = get_infix(curr.type); // 在经历上一行的advance()之后，这里的curr就是上面条件中的next。
        (this->*infix)();
    }
}

void Compiler::integer_expr() {
    long integer = std::stol(curr.lexeme);
    uint8_t index = Chunk::to_immediate(integer);
    if (index == 255) {
        emit_load_constant(integer);
    } else {
        emit_opcode(OpCode::LoadImmediate);
        emit_operand(index);
    }
}

void Compiler::float_expr() {
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

void Compiler::grouping_expr() {
    compile_expression();
    consume(TokenType::RIGHT_PAREN, "expect ) after the expression");
}

void Compiler::binary_expr() {
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

void Compiler::literal_expr() {
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
            FMT_ASSERT(false, "this line should be unreachable, but is actually reached");
    }
}

void Compiler::string_expr() {
    Value value = LoxObject::allocate<LoxString>(curr.lexeme.substr(1, curr.lexeme.size() - 2));
    emit_load_constant(std::move(value));
}

void Compiler::unary_expr() {
    TokenType type = curr.type;
    compile_precedence_at_least(Precedence::UNARY);
    if (type == TokenType::MINUS) {
        emit_opcode(OpCode::Negate);
    } else if (type == TokenType::BANG) {
        emit_opcode(OpCode::Not);
    }
}

void Compiler::variable_expr() {
    OperandSize key = current_chunk()->add_identifier(curr.lexeme);
    emit_opcode(OpCode::LoadGlobal);
    emit_operand_2(key);
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

OperandSize Compiler::parse_identifier() {
    consume(TokenType::IDENTIFIER, "expect an identifier here");
    return current_chunk()->add_identifier(curr.lexeme);
}

void Compiler::var_statement() {
    try {
        OperandSize key = parse_identifier();
        if (match(TokenType::EQUAL)) {
            compile_expression();
        } else {
            emit_opcode(OpCode::LoadNil);
        }
        consume();
        emit_opcode(OpCode::DefineGlobal);
        emit_operand_2(key);
    } catch (ConstantPoolOverflowError &err) {
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
