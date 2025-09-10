//
// Created by Yue Xue  on 9/3/25.
//

#include "typechecker/expression.h"

#include "typechecker/expressions/and_expression.h"
#include "typechecker/expressions/as_expression.h"
#include "typechecker/expressions/assignment_expression.h"
#include "typechecker/expressions/binary_expression.h"
#include "typechecker/expressions/call_expression.h"
#include "typechecker/expressions/dot_expression.h"
#include "typechecker/expressions/is_expression.h"
#include "typechecker/expressions/or_expression.h"
#include "typechecker/expressions/primary_expression.h"
#include "typechecker/expressions/unary_expression.h"

#include "typechecker/types/class_type.h"

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
        auto &next = tokens->peek_next();
        throw tokens->error_at(0, fmt::format("expect a token here, but got: '{}'",
            next.get_lexeme()));
    }
}

ExprPtr ExpressionParser::parse_call() {
    // a(b, c)
    // a.b.d
    auto left = parse_primary();
    while (tokens->match_one_of({TokenType::DOT, TokenType::LEFT_PAREN})) {
        const auto op = tokens->last_token();
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
            NOT_IMPLEMENTED();
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

ExprPtr ExpressionParser::parse_is() {
    auto left = parse_equality();
    while (tokens->match(TokenType::Is)) {
        auto test_type = type_parser->parse_type();
        left = std::make_unique<IsExpression>(std::move(left), test_type);
    }
    return left;
}

ExprPtr ExpressionParser::parse_and() {
    // a and b and c
    auto left = parse_is();
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

ExprPtr ExpressionParser::parse_as() {
    auto expr = parse_or();
    while (tokens->match(TokenType::As)) {
        auto as_type = type_parser->parse_type();
        expr = std::make_unique<AsExpression>(std::move(expr), as_type);
    }
    return expr;
}

ExprPtr ExpressionParser::parse_assignment() {
    auto left = parse_as();
    if (tokens->match(TokenType::EQUAL)) {
        // only parse the structure here
        // semantic errors like a + b = expr will be resolved at analysis time
        auto right = parse_assignment();
        left = std::make_unique<AssignmentExpression>(std::move(left), std::move(right));
    }
    return left;
}
