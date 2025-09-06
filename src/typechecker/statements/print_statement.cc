//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/statements/print_statement.h"

#include "typechecker/ast.h"
void PrintStatement::accept(AstCompiler &compiler) {
    compiler.visit_print_statement(this);
}
