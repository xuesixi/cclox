//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/statements/block_statement.h"

#include "typechecker/ast.h"

void BlockStatement::accept(AstCompiler &compiler) {
    compiler.visit_block_statement(this);
}
