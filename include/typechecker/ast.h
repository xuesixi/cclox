//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_AST_H
#define CCLOX_AST_H

#include "error.h"
#include "common.h"

#include "typechecker/statement.h"

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
    std::shared_ptr<LoxFunction> compile(std::string &&source) {
        StatementParser parser(std::move(source));
        statements = parser.parse_all();
    }

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

private:

    std::vector<StmtPtr> statements;
};

#endif //CCLOX_AST_H