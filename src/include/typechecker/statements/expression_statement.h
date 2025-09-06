//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_EXPRESSION_STATEMENT_H
#define CCLOX_EXPRESSION_STATEMENT_H

#include "typechecker/statement.h"

class ExpressionStatement: public Statement {
public:
    friend class AstCompiler;
    ExpressionStatement(ExprPtr expr): expr(std::move(expr)) {}
private:
    ExprPtr expr;
};

#endif //CCLOX_EXPRESSION_STATEMENT_H