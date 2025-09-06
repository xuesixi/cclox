//
// Created by Yue Xue  on 9/5/25.
//
#include "typechecker/ast.h"
#include "typechecker/expressions/unary_expression.h"

void AstCompiler::visit_unary_expr(UnaryExpression *expression) {
    auto type = expression->resolve_type();
    visit_expression(expression->operand.get());
    if (expression->op.get_type() == TokenType::MINUS) {

    }
}
