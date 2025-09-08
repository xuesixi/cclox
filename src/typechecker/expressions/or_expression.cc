//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/or_expression.h"
#include "typechecker/types/primitive_type.h"

#include "typechecker/ast.h"

TypePtr OrExpression::resolve_type(std::shared_ptr<ST_Scope> &scope) {
    auto left_type = left->resolve_type(scope);
    auto right_type = right->resolve_type(scope);
    this->line = left->get_line();
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
