//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_DOT_EXPRESSION_H
#define CCLOX_DOT_EXPRESSION_H

#include "typechecker/expression.h"

class DotExpression : public Expression {
public:
    friend class AstCompiler;
    DotExpression(ExprPtr target, const Token &target_field)
        : target(std::move(target)),
          target_field(std::move(target_field)) {
    }

    ExprPtr target;
    Token target_field;

    TypePtr resolve_type(std::shared_ptr<ST_Scope> &scope, TypePtr hint = nullptr) override;

    void accept(AstCompiler &compiler) override;

    ExpressionType get_expression_type() const override {
        return ExpressionType::Dot;
    }
};

#endif //CCLOX_DOT_EXPRESSION_H