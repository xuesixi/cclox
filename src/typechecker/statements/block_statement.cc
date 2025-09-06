//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/statements/block_statement.h"
#include "typechecker/types/primitive_type.h"

#include "typechecker/ast.h"

void BlockStatement::accept(AstCompiler &compiler) {
    compiler.visit_block_statement(this);
}

TypePtr BlockStatement::resolve_return_type() {
    for (auto &statement: body) {
        auto statement_return_type = statement->resolve_return_type();
        if (statement_return_type->type_enum != LoxTypeEnum::Unspecified) {
            return statement_return_type;
        }
    }
    return PrimitiveType::UnspecifiedType;
}
