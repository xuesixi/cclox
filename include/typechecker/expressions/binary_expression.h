//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_BINARY_EXPRESSION_H
#define CCLOX_BINARY_EXPRESSION_H

#include "typechecker/expression.h"


class BinaryExpression : public Expression {
public:
    friend class AstCompiler;
    BinaryExpression(ExprPtr left, ExprPtr right, const Token &op)
        : left(std::move(left)),
          right(std::move(right)),
          op(op) {
    }

    ExprPtr left;
    ExprPtr right;
    Token op;

    TypePtr resolve_type(std::shared_ptr<Scope> &scope) override;

    ExpressionType get_expression_type() const override {
        return ExpressionType::Binary;
    }

    void accept(AstCompiler &compiler) override;
};

#endif //CCLOX_BINARY_EXPRESSION_H