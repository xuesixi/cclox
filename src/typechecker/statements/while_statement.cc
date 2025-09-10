//
// Created by Yue Xue  on 9/9/25.
//

#include "typechecker/statements/while_statement.h"

#include "typechecker/ast.h"

TypePtr WhileStatement::resolve_return_type() {
    return body->resolve_return_type();
}

void WhileStatement::accept(AstCompiler &compiler) {
    compiler.visit_while_statement(this);
}
