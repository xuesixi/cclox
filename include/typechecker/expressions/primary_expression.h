//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_PRIMARY_EXPRESSION_H
#define CCLOX_PRIMARY_EXPRESSION_H

#include "typechecker/expression.h"


class PrimaryExpression : public Expression {
public:
    friend class AstCompiler;
    explicit PrimaryExpression(const Token &token) : value_token(token) {
    }

    Token value_token;

    /**
     * @copydoc Expression::resolve_type
     */
    TypePtr resolve_type() override;

    void accept(AstCompiler &compiler) override;
};

#endif //CCLOX_PRIMARY_EXPRESSION_H