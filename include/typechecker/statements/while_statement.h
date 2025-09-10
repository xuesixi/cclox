//
// Created by Yue Xue  on 9/9/25.
//

#ifndef CCLOX_WHILE_STATEMENT_H
#define CCLOX_WHILE_STATEMENT_H

#include "typechecker/statement.h"

class WhileStatement: public Statement {
public:
    friend class AstCompiler;
    WhileStatement(ExprPtr condition, StmtPtr body): condition(std::move(condition)), body(std::move(body)) {

    }
private:
    ExprPtr condition;
    StmtPtr body;

public:
    TypePtr resolve_return_type() override;

    void accept(AstCompiler &compiler) override;
};

#endif //CCLOX_WHILE_STATEMENT_H