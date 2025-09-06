//
// Created by Yue Xue  on 9/5/25.
//
#include "typechecker/ast.h"
#include "typechecker/expressions/unary_expression.h"

std::shared_ptr<LoxFunction> AstCompiler::compile(std::string &&source) {
    StatementParser parser(std::move(source));
    statements = parser.parse_all();
    return nullptr;
}

void AstCompiler::visit_unary_expr(UnaryExpression *expression) {
    auto type = expression->resolve_type();
    visit_expression(expression->operand.get());
    if (expression->op.get_type() == TokenType::MINUS) {

    }
}
