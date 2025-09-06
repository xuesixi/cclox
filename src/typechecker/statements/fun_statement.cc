//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/statements/fun_statement.h"

#include "typechecker/ast.h"
void FunStatement::accept(AstCompiler &compiler) {
    compiler.visit_fun_statement(this);
}
