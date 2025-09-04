//
// Created by Yue Xue  on 9/3/25.
//

#ifndef CCLOX_EXPRESSION_H
#define CCLOX_EXPRESSION_H

#include "scanner.h"
#include "tokenholder.h"

class Expression {
};

using ExprPtr = std::unique_ptr<Expression>;

class AssignmentExpression : public Expression {
public:
    AssignmentExpression(ExprPtr left, ExprPtr right)
        : left(std::move(left)),
          right(std::move(right)) {
    }

    ExprPtr left;
    ExprPtr right;
};

class OrExpression : public Expression {
public:
    OrExpression(ExprPtr left, ExprPtr right)
        : left(std::move(left)),
          right(std::move(right)) {
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
};

// class EqualityExpression : public Expression {
// public:
//     EqualityExpression(ExprPtr left, ExprPtr right, const Token &op)
//         : left(std::move(left)),
//           right(std::move(right)),
//           op(op) {
//     }
//
//     ExprPtr left;
//     ExprPtr right;
//     Token op;
// };
//
// class ComparisonExpression : public Expression {
// public:
//     ComparisonExpression(ExprPtr left, ExprPtr right, const Token &op)
//         : left(std::move(left)),
//           right(std::move(right)),
//           op(op) {
//     }
//
//     ExprPtr left;
//     ExprPtr right;
//     Token op;
// };
//
// class TermExpression : public Expression {
// public:
//     TermExpression(ExprPtr left, ExprPtr right, const Token &op)
//         : left(std::move(left)),
//           right(std::move(right)),
//           op(op) {
//     }
//
//     ExprPtr left;
//     ExprPtr right;
//     Token op;
// };
//
// class FactorExpression : public Expression {
// public:
//     FactorExpression(ExprPtr left, ExprPtr right, const Token &op)
//         : left(std::move(left)),
//           right(std::move(right)),
//           op(op) {
//     }
//
//     ExprPtr left;
//     ExprPtr right;
//     Token op;
// };
//
// class PowerExpression : public Expression {
// public:
//     PowerExpression(ExprPtr left, ExprPtr right)
//         : left(std::move(left)),
//           right(std::move(right)) {
//     }
//
//     ExprPtr left;
//     ExprPtr right;
// };

class UnaryExpression : public Expression {
public:
    UnaryExpression(ExprPtr operand, const Token &op)
        : operand(std::move(operand)),
          op(op) {
    }

    ExprPtr operand;
    Token op;
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
    ExpressionParser(std::shared_ptr<TokenHolder> &th) : tokens(th) {
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
