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

    TypePtr resolve_type() override;

    ExprPtr left;

    ExprPtr right;

    void accept(AstCompiler &compiler) override;
};
#endif //CCLOX_OR_EXPRESSION_H