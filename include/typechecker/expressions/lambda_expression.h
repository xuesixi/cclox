//
// Created by Yue Xue  on 9/10/25.
//

#ifndef CCLOX_LAMBDA_EXPRESSION_H
#define CCLOX_LAMBDA_EXPRESSION_H

#include "typechecker/expression.h"

class FunctionType;

class LambdaExpression : public Expression {
public:
    friend class AstCompiler;

    explicit LambdaExpression(ExprPtr return_expr) :
        expr(std::move(return_expr)),
        type_hint(nullptr) {}

    TypePtr resolve_type(std::shared_ptr<ST_Scope> &scope, TypePtr hint) override;

    void accept(AstCompiler &compiler) override;

    ExpressionType get_expression_type() const override;

private:
    std::shared_ptr<FunctionType> type_hint; // 默认为nullptr
    std::shared_ptr<ST_Scope> scope;
    uint8_t param_count = -1;
    ExprPtr expr;
};

#endif //CCLOX_LAMBDA_EXPRESSION_H
