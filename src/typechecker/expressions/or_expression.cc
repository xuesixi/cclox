//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/or_expression.h"
#include "typechecker/types/primitive_type.h"

#include "typechecker/ast.h"

TypePtr OrExpression::resolve_type(std::shared_ptr<ST_Scope> &scope, TypePtr hint) {
    const auto left_type = left->resolve_type(scope);
    const auto right_type = right->resolve_type(scope);
    this->line = left->get_line();
    if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Bool)) {
        return PrimitiveType::BoolType;
    } else {
        throw mismatch(left_type, right_type, "expect bool, but got '{}' and '{}'");
    }
}

void OrExpression::accept(AstCompiler &compiler) {
    compiler.visit_or_expr(this);
}
