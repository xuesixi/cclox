//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_OR_EXPRESSION_H
#define CCLOX_OR_EXPRESSION_H

#include "typechecker/expression.h"

/**
 * 只允许布尔表达式。
 */
class OrExpression : public Expression {
public:
    friend class AstCompiler;
    OrExpression(ExprPtr left, ExprPtr right)
        : left(std::move(left)),
          right(std::move(right)) {
    }

    TypePtr resolve_type(std::shared_ptr<ST_Scope> &scope, TypePtr hint = nullptr) override;

    ExprPtr left;

    ExprPtr right;

    void accept(AstCompiler &compiler) override;

    ExpressionType get_expression_type() const override {
        return ExpressionType::Or;
    }
};
#endif //CCLOX_OR_EXPRESSION_H