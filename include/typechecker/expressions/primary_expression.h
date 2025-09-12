//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_PRIMARY_EXPRESSION_H
#define CCLOX_PRIMARY_EXPRESSION_H

#include "typechecker/expression.h"


/**
 * 基础的表达式，包括以下几种可能：
 * - int, float, bool, nil, String, identifier(变量)
 */
class PrimaryExpression : public Expression {
public:
    friend class AstCompiler;
    explicit PrimaryExpression(const Token &token) : value_token(token) {
    }

    Token value_token;

    // todo: @
    TypePtr resolve_type(std::shared_ptr<ST_Scope> &scope, TypePtr hint = nullptr) override;

    void accept(AstCompiler &compiler) override;

    ExpressionType get_expression_type() const override {
        return ExpressionType::Primary;
    }
};

#endif //CCLOX_PRIMARY_EXPRESSION_H