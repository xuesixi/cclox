//
// Created by Yue Xue  on 9/9/25.
//

#ifndef CCLOX_IF_STATEMENT_H
#define CCLOX_IF_STATEMENT_H

#include "typechecker/statement.h"

class IfStatement: public Statement {
public:
    friend class AstCompiler;
    IfStatement(ExprPtr condition, StmtPtr then_branch, StmtPtr else_branch):
    condition(std::move(condition)),
    then_branch(std::move(then_branch)),
    else_branch(std::move(else_branch)) {}
private:
    ExprPtr condition;
    StmtPtr then_branch;
    StmtPtr else_branch;

public:
    TypePtr resolve_return_type() override;

    void accept(AstCompiler &compiler) override;
};
#endif //CCLOX_IF_STATEMENT_H