//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/assignment_expression.h"

#include "typechecker/ast.h"

TypePtr AssignmentExpression::resolve_type(std::shared_ptr<ST_Scope> &scope, TypePtr hint) {
    auto left_type = left->resolve_type(scope);
    const auto right_type = right->resolve_type(scope);
    this->line = left->get_line();
    if (left_type->accept(right_type) == false) {
        throw mismatch(left_type, right_type, "the assignment target is of type '{}' which cannot accept expression of type '{}'");
    }
    return left_type;
}

void AssignmentExpression::accept(AstCompiler &compiler) {
    compiler.visit_assignment_expr(this);
}
