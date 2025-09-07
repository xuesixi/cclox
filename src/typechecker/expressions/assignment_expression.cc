//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/assignment_expression.h"

#include "typechecker/ast.h"

TypePtr AssignmentExpression::resolve_type() {
    auto type = right->resolve_type();
    this->line = right->get_line();
    return type;
}

void AssignmentExpression::accept(AstCompiler &compiler) {
    compiler.visit_assignment_expr(this);
}
