//
// Created by Yue Xue  on 9/7/25.
//

#ifndef CCLOX_AS_EXPRESSION_H
#define CCLOX_AS_EXPRESSION_H

#include "typechecker/expression.h"

class AsExpression: public Expression {
public:
    AsExpression(ExprPtr expr, TypePtr &as_type): expr(std::move(expr)), as_type(as_type) {}
private:
    ExprPtr expr;
    TypePtr as_type;

public:
    TypePtr resolve_type(std::shared_ptr<Scope> &scope) override;

    void accept(AstCompiler &compiler) override;

    ExpressionType get_expression_type() const override;
};

#endif //CCLOX_AS_EXPRESSION_H