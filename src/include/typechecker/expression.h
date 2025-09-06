//
// Created by Yue Xue  on 9/3/25.
//

#ifndef CCLOX_EXPRESSION_H
#define CCLOX_EXPRESSION_H

#include "scanner.h"
#include "typechecker/typeparser.h"
#include "tokenholder.h"
#include "typechecker/types/primitive_type.h"
#include "types/function_type.h"

class Expression {
public:
    virtual ~Expression() {
    }

    /**
     * 检查并返回该表达式的返回值类型。
     * 如果出现了问题，抛出异常
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

    TypePtr resolve_type() override;

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

    TypePtr resolve_type() override;

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

    TypePtr resolve_type() override;
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

    TypePtr resolve_type() override;
};

class UnaryExpression : public Expression {
public:
    UnaryExpression(ExprPtr operand, const Token &op)
        : operand(std::move(operand)),
          op(op) {
    }

    ExprPtr operand;
    Token op;

    TypePtr resolve_type() override;
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

    TypePtr resolve_type() override;
};

class DotExpression : public Expression {
public:
    DotExpression(ExprPtr target, const Token &target_field)
        : target(std::move(target)),
          target_field(std::move(target_field)) {
    }

    ExprPtr target;
    Token target_field;

    TypePtr resolve_type() override;
};

class PrimaryExpression : public Expression {
public:
    explicit PrimaryExpression(const Token &token) : value_token(token) {
    }

    Token value_token;
    TypePtr resolve_type() override;
};

class ExpressionParser {
public:
    explicit ExpressionParser(std::shared_ptr<TokenHolder> &th) : tokens(th) {
    }

    ExprPtr parse_expression();

private:
    std::shared_ptr<TokenHolder> tokens;

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
};

#endif //CCLOX_EXPRESSION_H
