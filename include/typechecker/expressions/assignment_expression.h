//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_ASSIGNMENT_EXPRESSION_H
#define CCLOX_ASSIGNMENT_EXPRESSION_H

#include "typechecker/expression.h"

class AssignmentExpression : public Expression {
public:
    friend class AstCompiler;
    AssignmentExpression(ExprPtr left, ExprPtr right)
        : left(std::move(left)),
          right(std::move(right)) {
    }

    TypePtr resolve_type(std::shared_ptr<Scope> &scope) override;

    ExprPtr left;
    ExprPtr right;

    ExpressionType get_expression_type() const override {
        return ExpressionType::Assignment;
    }

    void accept(AstCompiler &compiler) override;
};

#endif //CCLOX_ASSIGNMENT_EXPRESSION_H