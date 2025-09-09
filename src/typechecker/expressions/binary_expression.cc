//
// Created by Yue Xue  on 9/5/25.
//

#include "typechecker/expressions/binary_expression.h"

#include "typechecker/ast.h"
#include "typechecker/types/primitive_type.h"

TypePtr BinaryExpression::resolve_type(std::shared_ptr<ST_Scope> &scope) {
    auto left_type = left->resolve_type(scope);
    auto right_type = right->resolve_type(scope);
    this->line = left->get_line();
    switch (op.get_type()) {
        case TokenType::MINUS:
        case TokenType::SLASH:
        case TokenType::STAR: {
            if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Int)) {
                return PrimitiveType::IntType;
            } else if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Float)) {
                return PrimitiveType::FloatType;
            } else if (LoxType::int_and_float(left_type, right_type)) {
                return PrimitiveType::FloatType;
            }
            throw mismatch(left_type, right_type, "expect int or float, but got '{}' and '{}'");
        }
        case TokenType::PLUS: {
            if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Int)) {
                return PrimitiveType::IntType;
            } else if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Float)) {
                return PrimitiveType::FloatType;
            } else if (LoxType::int_and_float(left_type, right_type)) {
                return PrimitiveType::FloatType;
            } else if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::String)) {
                return PrimitiveType::StringType;
            }
            throw mismatch(left_type, right_type, "expect int or float or two String, but got '{}' and '{}'");
        }
        case TokenType::STAR_STAR: {
            if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Int)
                || LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Float)
                || LoxType::int_and_float(left_type, right_type)) {
                return PrimitiveType::FloatType;
            }
            throw mismatch(left_type, right_type, "expect int or float, but got '{}' and '{}'");
        }
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
        case TokenType::EQUAL_EQUAL:
        case TokenType::BANG_EQUAL: {
            if (left_type->type_enum == right_type->type_enum) {
                return PrimitiveType::BoolType;
            }
            throw mismatch(left_type, right_type, "expect same type for comparison, but got '{}' and '{}'");
        }
        default:
            IMPL_ERROR("no such binary type");
    }
}

void BinaryExpression::accept(AstCompiler &compiler) {
    compiler.visit_binary_expr(this);
}
