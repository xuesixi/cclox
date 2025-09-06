//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/statements/expression_statement.h"

#include "typechecker/ast.h"
void ExpressionStatement::accept(AstCompiler &compiler) {
    compiler.visit_expression_statement(this);
}
