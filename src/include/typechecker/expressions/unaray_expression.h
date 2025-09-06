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

    TypePtr resolve_type() override;

    void accept(AstCompiler &compiler) override;
};

#endif //CCLOX_UNARAY_EXPRESSION_H