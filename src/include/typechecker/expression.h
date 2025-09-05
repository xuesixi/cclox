//
// Created by Yue Xue  on 9/3/25.
//

#ifndef CCLOX_EXPRESSION_H
#define CCLOX_EXPRESSION_H

#include "scanner.h"
#include "typechecker/typeparser.h"
#include "tokenholder.h"

class Expression {
public:
    virtual ~Expression() {
    }

    /**
     * 检查并返回该表达式的返回值类型。
     * 如果出现了问题，则返回Unspecified
     * @return 该表达式的返回值类型
     */
    virtual TypePtr resolve_type() = 0;
};

using ExprPtr = std::unique_ptr<Expression>;

class AssignmentExpression : public Expression {
public:
    AssignmentExpression(ExprPtr left, ExprPtr right)
        : left(std::move(left)),
          right(std::move(right)) {
    }

    TypePtr resolve_type() override {
        return right->resolve_type();
    }

    ExprPtr left;
    ExprPtr right;
};

/**
 * 只允许布尔表达式。
 */
class OrExpression : public Expression {
public:
    OrExpression(ExprPtr left, ExprPtr right)
        : left(std::move(left)),
          right(std::move(right)) {
    }

    TypePtr resolve_type() override {
        auto left_type = left->resolve_type();
        auto right_type = right->resolve_type();
        if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Bool)) {
            return PrimitiveType::BoolType;
        } else {
            throw MismatchedTypeError(fmt::format("expect bool, but got {} and {}", left_type->to_string(), right_type->to_string()));
        }
    }

    ExprPtr left;
    ExprPtr right;
};

class AndExpression : public Expression {
public:
    AndExpression(ExprPtr left, ExprPtr right)
        : left(std::move(left)),
          right(std::move(right)) {
    }

    ExprPtr left;
    ExprPtr right;

    TypePtr resolve_type() override {
        auto left_type = left->resolve_type();
        auto right_type = right->resolve_type();
        if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Bool)) {
            return PrimitiveType::BoolType;
        } else {
            throw MismatchedTypeError(fmt::format("expect bool, but got {} and {}", left_type->to_string(), right_type->to_string()));
        }
    }
};

class BinaryExpression : public Expression {
public:
    BinaryExpression(ExprPtr left, ExprPtr right, const Token &op)
        : left(std::move(left)),
          right(std::move(right)),
          op(op) {
    }

    ExprPtr left;
    ExprPtr right;
    Token op;

    TypePtr resolve_type() override {
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
                throw MismatchedTypeError(fmt::format("expect int or float, but got {} and {}", left_type->to_string(), right_type->to_string()));
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
                throw MismatchedTypeError(fmt::format("expect int or float or two String, but got {} and {}", left_type->to_string(), right_type->to_string()));
            }
            case TokenType::STAR_STAR: {
                if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Int)
                    || LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Float)
                    || LoxType::int_and_float(left_type, right_type)) {
                    return PrimitiveType::FloatType;
                }
                throw MismatchedTypeError(fmt::format("expect int or float, but got {} and {}", left_type->to_string(), right_type->to_string()));
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
                throw MismatchedTypeError(fmt::format("expect same type for comparison, but got {} and {}", left_type->to_string(), right_type->to_string()));
            }
            default:
                implementation_error("no such binary type");
                return PrimitiveType::MismatchedType;
        }
    }
};

class UnaryExpression : public Expression {
public:
    UnaryExpression(ExprPtr operand, const Token &op)
        : operand(std::move(operand)),
          op(op) {
    }

    ExprPtr operand;
    Token op;

    TypePtr resolve_type() override {
        auto operand_type = operand->resolve_type();
        if (op.get_type() == TokenType::BANG) {
            if (operand_type->type_enum == LoxTypeEnum::Bool) {
                return PrimitiveType::BoolType;
            } else {
                throw MismatchedTypeError(fmt::format("expect bool, but got {}", operand_type->to_string()));
            }
        } else if (op.get_type() == TokenType::MINUS) {
            if (operand_type->type_enum == LoxTypeEnum::Int ) {
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
};

class CallExpression : public Expression {
public:
    CallExpression(ExprPtr callee, std::vector<ExprPtr> &&arguments)
        : callee(std::move(callee)),
          arguments(std::move(arguments)) {
    }

    // run(a, b, c)
    ExprPtr callee;
    std::vector<ExprPtr> arguments;

    TypePtr resolve_type() override {
        auto type = callee->resolve_type();
        if (type->type_enum != LoxTypeEnum::Function) {
            throw MismatchedTypeError(fmt::format("expect a callable but got {}", type->to_string()));
        }
        std::shared_ptr<FunctionType> callable = std::static_pointer_cast<FunctionType>(type);
        if (callable->parameters.size() != arguments.size()) {
            throw MismatchedTypeError(fmt::format("the callable expects {} arguments, but got {}", callable->parameters.size(), arguments.size()));
        }
        for (size_t i = 0; i < arguments.size(); i ++) {

        }
    }
};

class DotExpression : public Expression {
public:
    DotExpression(ExprPtr target, const Token &target_field)
        : target(std::move(target)),
          target_field(std::move(target_field)) {
    }

    ExprPtr target;
    Token target_field;
};

class PrimaryExpression : public Expression {
public:
    explicit PrimaryExpression(const Token &token) : value_token(token) {
    }

    Token value_token;
};

class ExpressionParser {
public:
    explicit ExpressionParser(std::shared_ptr<TokenHolder> &th) : tokens(th) {
    }

    ExprPtr parse_expression();

    ExprPtr parse_primary();

    ExprPtr parse_call();

    ExprPtr parse_unary();

    ExprPtr parse_power();

    ExprPtr parse_factor();

    ExprPtr parse_term();

    ExprPtr parse_comparison();

    ExprPtr parse_equality();

    ExprPtr parse_and();

    ExprPtr parse_or();

    ExprPtr parse_assignment();

private:
    std::shared_ptr<TokenHolder> tokens;
};

#endif //CCLOX_EXPRESSION_H
