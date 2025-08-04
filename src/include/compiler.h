#ifndef CCLOX_COMPILER_H
#define CCLOX_COMPILER_H

#include "chunk.h"
#include "scanner.h"
#include <memory>
#include <string>

enum class Precedence {
    NONE,
    ASSIGNMENT,  // =
    OR,          // or
    AND,         // and
    EQUALITY,    // == !=
    COMPARISON,  // < > <= >=
    TERM,        // + -
    FACTOR,      // * /
    UNARY,       // ! -
    CALL,        // . ()
    PRIMARY
};

inline Precedence get_higher_precedence(Precedence precedence) {
    return static_cast<Precedence>(static_cast<int>(precedence) + 1);
}

class Compiler;

class Compiler {
public:
//    explicit Compiler();
    std::shared_ptr<Chunk> compile(std::string &&source);

private:
    /**
     * 这样的一个函数调用后，curr所在的token已经被解析，而下一个要被解析的token是next，因此，调用后面常常跟着advance()
     */
    using ParseFn = void (Compiler::*)();

    /**
     * 将目标opcode写入字节码中
     */
    void emit_opcode(OpCode op_code) {
        current_chunk->write_opcode(op_code, curr.line);
    }

    /**
     * 将目标操作数写入字节码中，可以正确地处理单双字节的操作数
     */
    void emit_operand(size_t operand) {
        current_chunk->write_operand(operand, curr.line);
    }

    void emit_return() {
        emit_opcode(OpCode::Return);
    }

    /**
     * 写入LoadConstant指令，将目标值置入常数池中，并将其索引作为操作数写入字节码中。该函数可以正确地处理uint16及以下的值，如果超出，则终止程序。
     */
    void emit_load_constant(Value &&value) {
        size_t index = current_chunk->add_constant(std::move(value));
        if (within<uint8_t>(index)) {
            emit_opcode(OpCode::LoadConstant8);
        } else if (within<uint16_t>(index)) {
            emit_opcode(OpCode::LoadConstant16);
        } else {
            error_at(curr, "constant pool overflow: too many constants!");
            std::abort();
        }
        emit_operand(index);
    }

    void end_compiler() {
        emit_return();
    }

    void error_at(const Token &token, const std::string &message);

    /**
     * 如果next是想要的token，则advance()。否则报错
     */
    void consume(TokenType type, const std::string &message);

    /**
     * 读取新的token，移动next和curr。如果新的token为error，那么一直读，知道遇到一个非error的token。
     */
    void advance();

    /*
     * 所有下面这些表达式解析函数，在运行后，curr的token已经被解析，下一个待解析的token是next，因此，一般在调用后，会再调用advance()
     */

    /**
     * 解析以next开始的连续的所有优先级大于等于at_least的表达式
     */
    void compile_precedence_at_least(Precedence at_least);

    /**
     * 解析从next开始的一个表达式。
     */
    void compile_expression();

    /**
     * 在已知curr是一个integer的时候调用
     */
    void integer_expr();

    /**
     * 在已知curr是一个float的时候调用，解析之.
     */
    void float_expr();

    /**
     * 在已知curr是左括号的时候调用
     */
    void grouping_expr();

    /**
     * 在已知curr是一个单元操作符的时候调用。
     */
    void unary_expr();

    /**
     * 在已知curr是一个二元操作符的时候调用。
     */
    void binary_expr();

    /**
     * 在已知curr是nil，true，false的时候调用
     */
    void literal_expr();

    /**
     * 在已知curr是字符串的时候调用
     */
    void string_expr();

    /**
     * 获取一个token的前缀函数（该token作为一个表达式的第一个token时的解析函数）。如果该token不能作为表达式的第一个token，则返回nullptr
     */
    static ParseFn get_prefix(TokenType type);

    /**
     * 获取一个token的中缀函数（该token用来连接两个表达式时的解析函数）。如果该token不能作为中缀，返回nullptr
     */
    static ParseFn get_infix(TokenType type);

    /**
     * 返回该token作为中缀时的运算优先级。
     */
    static Precedence get_precedence(TokenType type);

    std::unique_ptr<Scanner> scanner;

    Token next;  // 刚刚通过scanner得到的那个token，一般情况下，是下一个要检视的token
    Token curr; // next之前的那个token。之所以称之为curr，是因为大部份情况下，这才是我们正在检视的token
    bool has_error = false; // 编译的过程中是否出现过错误
    bool panic_mode = false; // 是否处于panic模式，出现错误时设置true，一路跳过到下一个statement，然后设置为false
    std::shared_ptr<Chunk> current_chunk;
};

#endif