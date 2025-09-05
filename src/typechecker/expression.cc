//
// Created by Yue Xue  on 9/3/25.
//

#include "typechecker/expression.h"

/**
 *
 * assignment
 * or
 * and
 * equality
 * comparison
 * term
 * factor
 * power
 * unary
 * primary
 *
 */

TypePtr AssignmentExpression::resolve_type() {
    return right->resolve_type();
}

TypePtr OrExpression::resolve_type() {
    auto left_type = left->resolve_type();
    auto right_type = right->resolve_type();
    if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Bool)) {
        return PrimitiveType::BoolType;
    } else {
        throw MismatchedTypeError(fmt::format("expect bool, but got {} and {}", left_type->to_string(),
                                              right_type->to_string()));
    }
}

TypePtr AndExpression::resolve_type() {
    auto left_type = left->resolve_type();
    auto right_type = right->resolve_type();
    if (LoxType::both_of_type(left_type, right_type, LoxTypeEnum::Bool)) {
        return PrimitiveType::BoolType;
    } else {
        throw MismatchedTypeError(fmt::format("expect bool, but got {} and {}", left_type->to_string(),
                                              right_type->to_string()));
    }
}

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

TypePtr CallExpression::resolve_type() {
    auto type = callee->resolve_type();
    if (type->type_enum != LoxTypeEnum::Function) {
        throw MismatchedTypeError(fmt::format("expect a callable but got {}", type->to_string()));
    }
    std::shared_ptr<FunctionType> callable = std::static_pointer_cast<FunctionType>(type);
    if (callable->parameters.size() != arguments.size()) {
        throw MismatchedTypeError(fmt::format("the callable expects {} arguments, but got {}",
                                              callable->parameters.size(), arguments.size()));
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        auto arg_type = arguments.at(i)->resolve_type();
        if (callable->parameters.at(i)->accept(arg_type) == false) {
            throw MismatchedTypeError(fmt::format("the {}th argument is expected to be {}, but got {}",
                                                  i + 1, callable->parameters.at(i)->to_string(), arg_type->to_string()));
        }
    }
    return callable->return_type;
}

ExprPtr ExpressionParser::parse_expression() {
    return parse_assignment();
}

ExprPtr ExpressionParser::parse_primary() {
    if (tokens->match(TokenType::LEFT_PAREN)) {
        // 括号优先级
        auto expr = parse_expression();
        tokens->consume(TokenType::RIGHT_PAREN, "expect a ')' to balance the parenthesis");
        return expr;
    }
    if (tokens->match_one_of({
        TokenType::INTEGER, TokenType::FLOAT, TokenType::Nil, TokenType::True, TokenType::False, TokenType::STRING, TokenType::FMT_STRING, TokenType::IDENTIFIER
    })) {
        return std::make_unique<PrimaryExpression>(tokens->last_token());
    } else {
        throw tokens->error_at(0, "expect a token as value here");
    }
}

ExprPtr ExpressionParser::parse_call() {
    // a(b, c)
    // a.b.d
    auto left = parse_primary();
    while (tokens->match_one_of({TokenType::DOT, TokenType::LEFT_PAREN})) {
        auto op = tokens->last_token();
        if (op.type == TokenType::DOT) {
            auto member = tokens->consume(TokenType::IDENTIFIER, "expect a member name here");
            left = std::make_unique<DotExpression>(std::move(left), member);
        } else if (op.type == TokenType::LEFT_PAREN) {
            std::vector<ExprPtr> arguments;
            if (tokens->match(TokenType::RIGHT_PAREN) == false) {
                do {
                    arguments.push_back(parse_expression());
                } while (tokens->match(TokenType::COMMA));
                tokens->consume(TokenType::RIGHT_PAREN, "expect a ')' to end the argument list");
            }
            left = std::make_unique<CallExpression>(std::move(left), std::move(arguments));
        } else {
            implementation_error("not implemented yet");
        }
    }
    return left;
}

ExprPtr ExpressionParser::parse_unary() {
    // --a
    if (tokens->match_one_of({TokenType::MINUS, TokenType::BANG})) {
        auto op = tokens->last_token();
        auto curr = parse_unary();
        return std::make_unique<UnaryExpression>(std::move(curr), op);
    } else {
        // return parse_primary();
        return parse_call();
    }
}

ExprPtr ExpressionParser::parse_power() {
    auto left = parse_unary();

    if (tokens->match_one_of({TokenType::STAR_STAR})) {
        auto op = tokens->last_token();
        // 指数是右结合的
        auto right = parse_power();
        left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), op);
    }
    return left;
}

ExprPtr ExpressionParser::parse_factor() {
    auto left = parse_power();

    // a / b ** c * b

    while (tokens->match_one_of({TokenType::STAR, TokenType::SLASH})) {
        auto op = tokens->last_token();
        auto right = parse_power();
        left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), op);
    }
    return left;
}

ExprPtr ExpressionParser::parse_term() {
    auto left = parse_factor();

    while (tokens->match_one_of({TokenType::PLUS, TokenType::MINUS})) {
        auto op = tokens->last_token();
        auto right = parse_factor();
        left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), op);
    }
    return left;
}

ExprPtr ExpressionParser::parse_comparison() {
    // a > c + 2 < 3
    auto left = parse_term();
    while (tokens->match_one_of({
        TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQUAL, TokenType::GREATER_EQUAL
    })) {
        auto op = tokens->last_token();
        auto right = parse_term();
        left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), op);
    }
    return left;
}

ExprPtr ExpressionParser::parse_equality() {
    // a < d == b > c == e
    auto left = parse_comparison();
    while (tokens->match_one_of({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
        auto op = tokens->last_token();
        auto right = parse_comparison();
        left = std::make_unique<BinaryExpression>(std::move(left), std::move(right), op);
    }
    return left;
}

ExprPtr ExpressionParser::parse_and() {
    // a and b and c
    auto left = parse_equality();
    if (tokens->match(TokenType::And)) {
        auto right = parse_and();
        return std::make_unique<AndExpression>(std::move(left), std::move(right));
    }
    return left;
}

ExprPtr ExpressionParser::parse_or() {
    auto left = parse_and();
    if (tokens->match(TokenType::Or)) {
        auto right = parse_or();
        return std::make_unique<OrExpression>(std::move(left), std::move(right));
    }
    return left;
}

ExprPtr ExpressionParser::parse_assignment() {
    auto left = parse_or();
    if (tokens->match(TokenType::EQUAL)) {
        // only parse the structure here
        // semantic errors like a + b = expr will be resolved at analysis time
        auto right = parse_assignment();
        left = std::make_unique<AssignmentExpression>(std::move(left), std::move(right));
    }
    return left;
}
