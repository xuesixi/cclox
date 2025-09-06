//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_CALL_EXPRESSION_H
#define CCLOX_CALL_EXPRESSION_H

#include "typechecker/expression.h"

class CallExpression : public Expression {
public:
    friend class AstCompiler;
    CallExpression(ExprPtr callee, std::vector<ExprPtr> &&arguments)
        : callee(std::move(callee)),
          arguments(std::move(arguments)) {
    }

    // run(a, b, c)
    ExprPtr callee;
    std::vector<ExprPtr> arguments;

    TypePtr resolve_type() override;

    void accept(AstCompiler &compiler) override;
};

#endif //CCLOX_CALL_EXPRESSION_H