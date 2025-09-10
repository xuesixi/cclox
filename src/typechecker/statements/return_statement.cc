//
// Created by Yue Xue  on 9/8/25.
//

#include "typechecker/statements/return_statement.h"
#include "typechecker/ast.h"
#include "typechecker/types/primitive_type.h"

TypePtr ReturnStatement::resolve_return_type() {
    if (value == nullptr) {
        return PrimitiveType::UnspecifiedType;
    }
    return cached_return_type;
}

void ReturnStatement::accept(AstCompiler &compiler) {
    compiler.visit_return_statement(this);
}
