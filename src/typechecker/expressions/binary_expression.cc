//
// Created by Yue Xue  on 9/5/25.
//

#include "../../include/typechecker/expressions/binary_expression.h"

#include "typechecker/ast.h"
#include "typechecker/types/primitive_type.h"

TypePtr BinaryExpression::resolve_type() {
    auto left_type = left->resolve_type();
    auto right_type = right->resolve_type();
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
            throw MismatchedTypeError(fmt::format("expect int or float, but got {} and {}", left_type->to_string(),
                                                  right_type->to_string()));
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
            throw MismatchedTypeError(fmt::format("expect int or float or two String, but got {} and {}",
                                                  left_type->to_string(), right_type->to_string()));
        }
        case TokenType::STAR_STAR: {
            if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Int)
                || LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Float)
                || LoxType::int_and_float(left_type, right_type)) {
                return PrimitiveType::FloatType;
            }
            throw MismatchedTypeError(fmt::format("expect int or float, but got {} and {}", left_type->to_string(),
                                                  right_type->to_string()));
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
            throw MismatchedTypeError(fmt::format("expect same type for comparison, but got {} and {}",
                                                  left_type->to_string(), right_type->to_string()));
        }
        default:
            implementation_error("no such binary type");
            return PrimitiveType::MismatchedType;
    }
}

void BinaryExpression::accept(AstCompiler &compiler) {
    compiler.visit_binary_expr(this);
}
