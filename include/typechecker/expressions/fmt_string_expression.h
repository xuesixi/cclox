//
// Created by Yue Xue  on 9/11/25.
//

#ifndef CCLOX_FMT_STRING_EXPRESSION_H
#define CCLOX_FMT_STRING_EXPRESSION_H

#include "typechecker/expression.h"
#include <string>
#include <tuple>

class FmtStringExpression: public Expression {
public:
    friend class AstCompiler;
    FmtStringExpression(std::vector<ExprPtr> expressions): expressions(std::move(expressions)) {

    }
private:
    Token token;
    std::vector<ExprPtr> expressions;

public:
    TypePtr resolve_type(std::shared_ptr<ST_Scope> &scope, TypePtr hint) override;

    void accept(AstCompiler &compiler) override;

    ExpressionType get_expression_type() const override;
};

#endif //CCLOX_FMT_STRING_EXPRESSION_H