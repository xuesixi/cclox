//
// Created by Yue Xue  on 9/9/25.
//

#include "typechecker/expressions/is_expression.h"

#include "typechecker/ast.h"
#include "typechecker/types/primitive_type.h"


TypePtr IsExpression::resolve_type(std::shared_ptr<ST_Scope> &scope) {
    value->resolve_type(scope);
    line = value->get_line();
    return PrimitiveType::BoolType;
}

void IsExpression::accept(AstCompiler &compiler) {
    compiler.visit_is_expr(this);
}

Expression::ExpressionType IsExpression::get_expression_type() const {
    return ExpressionType::Is;
}
