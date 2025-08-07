#ifndef CCLOX_COMPILER_H
#define CCLOX_COMPILER_H

#include "chunk.h"
#include "common.h"
#include "scanner.h"
#include "scope.h"
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
    std::shared_ptr<Chunk> compile(std::string &&source);

private:
    /**
     * 这样的一个函数调用后，curr所在的token已经被解析，而下一个要被解析的token是next，因此，调用后面常常跟着advance()
     */
    using ParseFn = void (Compiler::*)([[maybe_unused]] bool);

    /**
     * 将目标opcode写入字节码中
     */
    void emit_opcode(OpCode op_code) {
        current_chunk()->write_opcode(op_code, curr.line);
    }

    /**
     * 将目标操作数写入字节码中，根据operand的值写入一个或者两个字节。超出uint16则是实现错误
     */
    void emit_operand(OperandSize operand) {
        current_chunk()->write_operand(operand, curr.line);
    }

    /**
     * 将目标操作数写入字节码中，无论operand的值，总是写入两个字节。超出uint16则是实现错误
     */
    void emit_operand_2(OperandSize operand) {
        current_chunk()->write_operand_2(operand, curr.line);
    }

    void emit_return() {
        emit_opcode(OpCode::Return);
    }

    /**
     * 写入LoadConstant指令，将目标值置入常数池中，并将其索引作为操作数写入字节码中。该函数可以正确地处理uint16及以下的值，如果超出，则error_at(curr)
     */
    void emit_load_constant(Value &&value);

    void end_compiler() {
        emit_return();
    }

    /**
     * 表明指定的token处出现了编译问题。如果原本不处于panic_mode，则会输出token元数据和错误消息，并设置panic_mode以及hash_error。
     * 如果原本已经处于panic_mode，则什么都不做，立刻返回（这是为了防止在下一次synchronize之前出现大量的令人疑惑的错误消息）
     */
    void error_at(const Token &token, const std::string &message);

    /**
     * 如果next是想要的token，则advance()。否则error-at。
     * type 的默认参数是semicolon，消息的默认参数是"expect a ';' to end the statement"。
     * 在repl模式下不会报错，而是抛出ConsumePending。
     */
    void consume(TokenType type, const std::string &message);

    /**
     * 读取新的token，移动next和curr。如果新的token为error，那么一直读，直到遇到一个非error的token。
     */
    void advance();

    /**
     * 检查next的类型是否为type
     */
    bool check(TokenType type) {
        return next.type == type;
    }

    /**
     * 如果next的类型不为type，返回false，否则advance()（这意味着curr的类型为指定的type，而next将会是下一个要解析的token），并返回true
     */
    bool match(TokenType type) {
        if (!check(type)) {
            return false;
        }
        advance();
        return true;
    }

    /**
     * 当遇到编译错误后，调用该函数来到达到下一个语句的开头。
     * 这里的语句是模糊的说法，只需要满足下面两个条件之一：
     * 1. curr是分号。这意味着一个语句在curr结束。
     * 2. next是class，fun，var，while之类的token。这意味着下一个语句在next开始。
     * 无论哪一种情况，在该函数调用后，下一个要解析的token都是next。且退出panic_mode
     */
    void synchronize();

    /**
     * 解析next开始的下一个语句
     */
    void declaration();

    /**
     * 解析next开始的下一个非申明语句
     */
    void statement();

    /**
     * 在已知curr为var的时候调用。
     */
    void var_statement();

    /**
     * 将next解析为一个identifier，将其lexeme添加到标识符池中，返回其键.
     * 如果该函数正常返回，则返回值必然处于uint16范围内，否则会抛出异常.
     * 调用后，下一个待解析的token为next.
     */
    OperandSize resolve_global_identifier();

    /**
     * 在curr已经被判定为print的时候调用。
     */
    void print_statement();

    /**
     * 在curr已知是if的时候使用
     */
    void if_statement();

    /**
     * 在curr已知是左大括号的时候调用。该函数自身没有涉及scope的处理
     */
    void block_statement();

    /**
     * 解析next开始的下一个表达式语句。
     */
    void expression_statement();

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
     * 在已知curr是一个identifier的时候调用
     */
    void variable_expr(bool can_assign);

    /**
     * 在已知curr是一个integer的时候调用
     */
    void integer_expr(bool can_assign);

    /**
     * 在已知curr是一个float的时候调用，解析之.
     */
    void float_expr(bool can_assign);

    /**
     * 在已知curr是左括号的时候调用
     */
    void grouping_expr(bool can_assign);

    /**
     * 在已知curr是一个单元操作符的时候调用。
     */
    void unary_expr(bool can_assign);

    /**
     * 在已知curr是一个二元操作符的时候调用。
     */
    void binary_expr(bool can_assign);
    
    /**
     * 在已知curr是and的时候调用。
     */
    void and_expr(bool can_assign);

    /**
     * 在已知curr是or的时候调用。
     */
    void or_expr(bool can_assign);

    /**
     * 在已知curr是nil，true，false的时候调用
     */
    void literal_expr(bool can_assign);

    /**
     * 在已知curr是字符串的时候调用
     */
    void string_expr(bool can_assign);

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

    /**
     * 向字节码中写入 [jump, placeholder1, placeholder2]。返回placeholder1的索引。该函数需要配合patch函数使用
     */
    size_t emit_jump(OpCode jump_instruction);

    /**
     * @param from_label 一个由emit_jump返回的标签（偏移值）
     *
     * 修改参数所代表的那个jump指令的操作数，使它跳转到这里。
     */
    void patch_jump(size_t from_label);

    std::shared_ptr<Chunk> current_chunk() {
        return chunk;
    }

    std::unique_ptr<Scanner> scanner;
    Token next;  // 刚刚通过scanner得到的那个token，一般情况下，是下一个要检视的token
    Token curr; // next之前的那个token。之所以称之为curr，是因为大部份情况下，这才是我们正在检视的token
    bool has_error = false; // 编译的过程中是否出现过错误
    bool panic_mode = false; // 是否处于panic模式，出现错误时设置true，一路跳过到下一个statement，然后设置为false
    std::shared_ptr<Chunk> chunk;
    std::shared_ptr<Scope> scope;
};

#endif