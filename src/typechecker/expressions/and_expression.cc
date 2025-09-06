//
// Created by Yue Xue  on 9/5/25.
//

#include "../../include/typechecker/expressions/and_expression.h"

#include "typechecker/ast.h"

TypePtr AndExpression::resolve_type() {
    auto left_type = left->resolve_type();
    auto right_type = right->resolve_type();
    if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Bool)) {
        return PrimitiveType::BoolType;
    } else {
        throw MismatchedTypeError(fmt::format("expect bool, but got {} and {}", left_type->to_string(),
                                              right_type->to_string()));
    }
}

void AndExpression::accept(AstCompiler &compiler) {
    compiler.visit_and_expr(this);
}
