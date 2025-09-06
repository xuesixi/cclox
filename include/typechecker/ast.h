//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_AST_H
#define CCLOX_AST_H

#include "error.h"
#include "common.h"
#include "chunk.h"

#include "typechecker/statement.h"

class Scope;
class BlockStatement;
class PrintStatement;
class VarStatement;
class ExpressionStatement;
class FunStatement;
class LoxFunction;

class AndExpression;
class AssignmentExpression;
class BinaryExpression;
class CallExpression;
class DotExpression;
class OrExpression;
class PrimaryExpression;
class UnaryExpression;

class AstCompiler {
public:
    std::shared_ptr<LoxFunction> compile(std::string &&source);

    void visit_statement(Statement *stmt) {
        stmt->accept(*this);
    }

    void visit_expression(Expression *expr) {
        expr->accept(*this);
    }

    void visit_block_statement(BlockStatement *block_statement) {

    }

    void visit_expression_statement(ExpressionStatement *expression_statement) {

    }

    void visit_fun_statement(FunStatement *fun_statement) {

    }

    void visit_var_statement(VarStatement *var_statement) {

    }

    void visit_print_statement(PrintStatement *print_statement) {

    }

    void visit_and_expr(AndExpression *expression) {

    }

    void visit_assignment_expr(AssignmentExpression *expression) {

    }

    void visit_binary_expr(BinaryExpression *expression) {

    }

    void visit_call_expr(CallExpression *expression) {

    }

    void visit_dot_expr(DotExpression *expression) {

    }

    void visit_or_expr(OrExpression *expression) {

    }

    void visit_primary_expr(PrimaryExpression *expression) {

    }

    void visit_unary_expr(UnaryExpression *expression);

    Chunk &current_chunk();

private:

    // /**
    //  * 将目标opcode写入字节码中
    //  */
    // void emit_opcode(Opcode op_code, int line);
    //
    // /**
    //  * 将目标操作数写入字节码中，根据operand的值写入一个或者两个字节。
    //  * @pre operand处在uint16范围内
    //  */
    // void emit_operand_flexible(size_t operand, int line);
    //
    // /**
    //  * 将目标操作数写入字节码中，只写入一个字节。调用者需要保证参数是uint8。
    //  * @pre operand处在uint8范围内
    //  */
    // void emit_operand_1(size_t operand, int line);
    //
    // /**
    //  * 将目标操作数写入字节码中，无论operand的值，总是写入两个字节。超出uint16则是实现错误
    //  * @pre operand 处在uint16范围内
    //  */
    // void emit_operand_2(size_t operand, int line);

    std::vector<StmtPtr> statements;
    std::shared_ptr<Scope> scope;
};

#endif //CCLOX_AST_H