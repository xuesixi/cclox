//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/assignment_expression.h"

#include "typechecker/ast.h"

TypePtr AssignmentExpression::resolve_type(std::shared_ptr<Scope> &scope) {
    auto left_type = left->resolve_type(scope);
    auto right_type = right->resolve_type(scope);
    if (left_type->accept(right_type) == false) {
        throw MismatchedTypeError(fmt::format("the assignment target is of type: {} which cannot accept expression of type: {}", left_type->to_string(), right_type->to_string()));
    }
    this->line = left->get_line();
    return left_type;
}

void AssignmentExpression::accept(AstCompiler &compiler) {
    compiler.visit_assignment_expr(this);
}
