//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/unaray_expression.h"

#include "typechecker/types/primitive_type.h"
#include "typechecker/ast.h"

TypePtr UnaryExpression::resolve_type() {
    auto operand_type = operand->resolve_type();
    if (op.get_type() == TokenType::BANG) {
        if (operand_type->type_enum == LoxTypeEnum::Bool) {
            return PrimitiveType::BoolType;
        } else {
            throw MismatchedTypeError(fmt::format("expect bool, but got {}", operand_type->to_string()));
        }
    } else if (op.get_type() == TokenType::MINUS) {
        if (operand_type->type_enum == LoxTypeEnum::Int) {
            return PrimitiveType::IntType;
        } else if (operand_type->type_enum == LoxTypeEnum::Float) {
            return PrimitiveType::FloatType;
        }
        throw MismatchedTypeError(fmt::format("expect int or float, but got {}", operand_type->to_string()));
    } else {
        implementation_error("unary operand that is neither ! nor -");
        return PrimitiveType::MismatchedType;
    }
}

void UnaryExpression::accept(AstCompiler &compiler) {
    compiler.visit_unary_expr(this);
}
