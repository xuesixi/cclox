//
// Created by Yue Xue  on 9/9/25.
//

#include "typechecker/statements/if_statement.h"

#include "typechecker/ast.h"
#include "typechecker/types/primitive_type.h"

TypePtr IfStatement::resolve_return_type() {
    auto then_type = then_branch->resolve_return_type();
    const auto else_type = else_branch->resolve_return_type();
    if (then_type->type_enum != LoxTypeEnum::Unspecified && else_type->type_enum != LoxTypeEnum::Unspecified) {
        return then_type;
    }
    return PrimitiveType::UnspecifiedType;
}

void IfStatement::accept(AstCompiler &compiler) {
    compiler.visit_if_statement(this);
}
