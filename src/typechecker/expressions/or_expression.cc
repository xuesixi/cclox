//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/or_expression.h"
#include "typechecker/types/primitive_type.h"

#include "typechecker/ast.h"

TypePtr OrExpression::resolve_type() {
    auto left_type = left->resolve_type();
    auto right_type = right->resolve_type();
    if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Bool)) {
        return PrimitiveType::BoolType;
    } else {
        throw MismatchedTypeError(fmt::format("expect bool, but got {} and {}", left_type->to_string(),
                                              right_type->to_string()));
    }
}

void OrExpression::accept(AstCompiler &compiler) {
    compiler.visit_or_expr(this);
}
