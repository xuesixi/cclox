//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_PRINT_STATEMENT_H
#define CCLOX_PRINT_STATEMENT_H

#include "typechecker/statement.h"

class PrintStatement: public Statement {
public:
    friend class AstCompiler;
    PrintStatement(ExprPtr expr): expr(std::move(expr)) {

        static_assert(!std::is_abstract_v<PrintStatement>);
    }

    void accept(AstCompiler &compiler) override;

private:
    ExprPtr expr;
};

#endif //CCLOX_PRINT_STATEMENT_H