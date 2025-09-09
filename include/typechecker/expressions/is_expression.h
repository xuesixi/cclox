//
// Created by Yue Xue  on 9/9/25.
//

#ifndef CCLOX_IS_EXPRESSION_H
#define CCLOX_IS_EXPRESSION_H

#include "typechecker/expression.h"

class IsExpression: public Expression {
public:
    friend class AstCompiler;
    IsExpression(ExprPtr &&value, const TypePtr &type): value(std::move(value)), test_type(type) {

    }

    TypePtr resolve_type(std::shared_ptr<ST_Scope> &scope) override;

    void accept(AstCompiler &compiler) override;

    ExpressionType get_expression_type() const override;

private:
    TypePtr test_type;
    ExprPtr value;
};

#endif //CCLOX_IS_EXPRESSION_H