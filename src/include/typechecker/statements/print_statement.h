//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_PRINT_STATEMENT_H
#define CCLOX_PRINT_STATEMENT_H

#include "typechecker/statement.h"

class PrintStatement: public Statement {
public:
    PrintStatement(ExprPtr expr): expr(std::move(expr)) {

    }
private:
    ExprPtr expr;
};

#endif //CCLOX_PRINT_STATEMENT_H