//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/and_expression.h"

#include "typechecker/ast.h"
#include "typechecker/types/primitive_type.h"

TypePtr AndExpression::resolve_type(std::shared_ptr<ST_Scope> &scope) {
    auto left_type = left->resolve_type(scope);
    auto right_type = right->resolve_type(scope);
    this->line = left->get_line();
    if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Bool)) {
        return PrimitiveType::BoolType;
    } else {
        throw mismatch(left_type, right_type, "expect bool, but got {} and {}");
    }
}

void AndExpression::accept(AstCompiler &compiler) {
    compiler.visit_and_expr(this);
}
