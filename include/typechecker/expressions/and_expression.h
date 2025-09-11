//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_AND_EXPRESSION_H
#define CCLOX_AND_EXPRESSION_H

#include "typechecker/expression.h"

class AndExpression : public Expression {
public:
    friend class AstCompiler;
    AndExpression(ExprPtr left, ExprPtr right)
        : left(std::move(left)),
          right(std::move(right)) {
    }

    ExprPtr left;
    ExprPtr right;

    TypePtr resolve_type(std::shared_ptr<ST_Scope> &scope, TypePtr hint = nullptr) override;

    ExpressionType get_expression_type() const override {
        return ExpressionType::And;
    }

    void accept(AstCompiler &compiler) override;
};

#endif //CCLOX_AND_EXPRESSION_H