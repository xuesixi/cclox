//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/statements/var_statement.h"

#include "typechecker/ast.h"
void VarStatement::accept(AstCompiler &compiler) {
    compiler.visit_var_statement(this);
}
