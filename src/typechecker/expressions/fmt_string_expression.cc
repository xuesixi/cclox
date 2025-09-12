//
// Created by Yue Xue  on 9/11/25.
//

#include "typechecker/expressions/fmt_string_expression.h"

#include "typechecker/ast.h"
#include "typechecker/expressions/primary_expression.h"
#include "typechecker/types/primitive_type.h"

TypePtr FmtStringExpression::resolve_type(std::shared_ptr<ST_Scope> &scope, TypePtr hint) {
    for (auto & expression : expressions) {
        expression->resolve_type(scope);
    }
    line = expressions.at(0)->get_line();
    return PrimitiveType::StringType;
}

void FmtStringExpression::accept(AstCompiler &compiler) {
    compiler.visit_fmt_string_expr(this);
}

Expression::ExpressionType FmtStringExpression::get_expression_type() const {
    return ExpressionType::FmtString;
}
