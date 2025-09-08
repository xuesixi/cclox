//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_UNARAY_EXPRESSION_H
#define CCLOX_UNARAY_EXPRESSION_H

#include "typechecker/expression.h"

class UnaryExpression : public Expression {
public:
    friend class AstCompiler;
    UnaryExpression(ExprPtr operand, const Token &op)
        : operand(std::move(operand)),
          op(op) {
    }

    ExprPtr operand;
    Token op;

    TypePtr resolve_type(std::shared_ptr<Scope> &scope) override;

    void accept(AstCompiler &compiler) override;

    ExpressionType get_expression_type() const override {
        return ExpressionType::Unary;
    }
};

#endif //CCLOX_UNARAY_EXPRESSION_H