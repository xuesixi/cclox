#include "compiler.h"
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

void Compiler::consume(TokenType type, const std::string &message) {
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
    //todo
    scanner = std::make_unique<Scanner>(std::move(source));
    current_chunk = std::make_shared<Chunk>();
    advance();
    compile_expression();
    end_compiler();
    if (has_error) {
        return nullptr;
    } else {
        return current_chunk;
    }
}


Compiler::ParseFn Compiler::get_prefix(TokenType type) {
    switch (type) {
        case TokenType::LEFT_PAREN:
            return &Compiler::compile_grouping;
        case TokenType::MINUS:
        case TokenType::BANG:
            return &Compiler::compile_unary;
        case TokenType::INTEGER:
            return &Compiler::compile_integer;
        case TokenType::FLOAT:
            return &Compiler::compile_float;
        case TokenType::NIL:
        case TokenType::TRUE:
        case TokenType::FALSE:
            return &Compiler::compile_literal;
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
            return &Compiler::compile_binary;
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

void Compiler::compile_integer() {
    long integer = std::stol(curr.lexeme);
    emit_load_constant(integer);
}

void Compiler::compile_float() {
    double decimal = std::stod(curr.lexeme);
    emit_load_constant(decimal);
}

void Compiler::compile_expression() {
    compile_precedence_at_least(Precedence::ASSIGNMENT);
}

void Compiler::compile_grouping() {
    compile_expression();
    consume(TokenType::RIGHT_PAREN, "expect ) after the expression");
}

void Compiler::compile_binary() {
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

void Compiler::compile_literal() {
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
    }
}

void Compiler::compile_unary() {
    TokenType type = curr.type;
    compile_precedence_at_least(Precedence::UNARY);
    if (type == TokenType::MINUS) {
        emit_opcode(OpCode::Negate);
    } else if (type == TokenType::BANG) {
        emit_opcode(OpCode::Not);
    }
}
