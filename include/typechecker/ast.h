//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_AST_H
#define CCLOX_AST_H

#include "error.h"
#include "common.h"
#include "chunk.h"
#include "statements/block_statement.h"

#include "typechecker/statement.h"

class WhileStatement;
class IfStatement;
class IsExpression;
class ST_Scope;
class BlockStatement;
class PrintStatement;
class VarStatement;
class ExpressionStatement;
class FunStatement;
class ReturnStatement;


class LoxFunction;
class LoxClosure;

class AndExpression;
class AsExpression;
class AssignmentExpression;
class BinaryExpression;
class CallExpression;
class DotExpression;
class OrExpression;
class PrimaryExpression;
class UnaryExpression;

class AstCompiler {
public:
    /**
     * 编译给定的源代码, 该函数内部会捕获编译期间的所有错误。
     * @param source 源代码
     * @return 如果一切正常，返回 main 函数。如果出现编译错误，返回空指针
     */
    std::shared_ptr<LoxClosure> compile(std::string &&source);

    /**
     * 编译指定的语句。该函数内部会捕获编译错误，并设置 has_error
     */
    void visit_statement(const StmtPtr &stmt) {
        try {
            stmt.get()->accept(*this);
        } catch (CompilerError &error) {
            has_error = true;
            std::cerr << error.what() << std::endl;
        }
    }

    /**
     * 访问一个表达式，生成对应的代码。
     * 该函数会假设表达式的类型已经在事前被验证过了。
     */
    void visit_expression(const ExprPtr &expr) {
        expr.get()->accept(*this);
    }

    void visit_block_statement(BlockStatement *stmt);

    void visit_expression_statement(ExpressionStatement *stmt);

    void visit_fun_statement(FunStatement *fun);

    void visit_var_statement(VarStatement *stmt);

    void visit_print_statement(PrintStatement *stmt);

    void visit_return_statement(ReturnStatement *return_statement);

    void visit_if_statement(IfStatement *if_statement);

    void visit_while_statement(WhileStatement *while_statement);

    // ----------------分割线------------------------

    void visit_and_expr(AndExpression *expr);

    void visit_as_expr(AsExpression *expr);

    void visit_is_expr(IsExpression *expr);

    void visit_assignment_expr(AssignmentExpression *expr);

    void visit_binary_expr(BinaryExpression *expr);

    void visit_call_expr(CallExpression *expr);

    void visit_dot_expr(DotExpression *expr) {

        NOT_IMPLEMENTED();
    }

    void visit_or_expr(OrExpression *expr);

    void visit_primary_expr(PrimaryExpression *expr);

    void visit_unary_expr(UnaryExpression *expr);

private:

    Chunk &current_chunk();

    /**
     * 将目标opcode写入字节码中
     */
    void emit_opcode(Opcode op_code, int line);

    /**
     * 将目标操作数写入字节码中，根据operand的值写入一个或者两个字节。
     * @pre operand处在uint16范围内
     */
    void emit_operand_flexible(size_t operand, int line);

    /**
     * 将目标操作数写入字节码中，只写入一个字节。调用者需要保证参数是uint8。
     * @pre operand处在uint8范围内
     */
    void emit_operand_1(size_t operand, int line);

    /**
     * 将目标操作数写入字节码中，无论operand的值，总是写入两个字节。超出uint16则是实现错误
     * @pre operand 处在uint16范围内
     */
    void emit_operand_2(size_t operand, int line);

    /**
     * 将一个 value 添加入常数池，根据其索引生成 LoadConstant 或者 LoadConstant2
     */
    void emit_constant(const Value &value, int line);

    OperandSize emit_jump(Opcode jump_operation, int line);

    void patch_jump(OperandSize from_label);

    void loop_back(size_t destination);

    bool is_global_scope() const;

    std::vector<StmtPtr> statements;
    std::shared_ptr<ST_Scope> scope;
    std::shared_ptr<LoxClosure> main;
    bool has_error = false;
};

#endif //CCLOX_AST_H