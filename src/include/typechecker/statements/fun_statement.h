//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_FUN_STATEMENT_H
#define CCLOX_FUN_STATEMENT_H

#include "typechecker/statement.h"

class FunStatement: public Statement {
public:
    friend class AstCompiler;
    FunStatement(const Token &fun_name, std::vector<std::pair<Token, TypePtr>> &&params, TypePtr &type, std::vector<StmtPtr> &&body)
    : fun_name(fun_name), return_type(type), parameters(std::move(params)), body(std::move(body)) {

        static_assert(!std::is_abstract_v<FunStatement>);
    }

    void accept(AstCompiler &compiler) override;

private:
    TypePtr return_type;
    Token fun_name;
    std::vector<std::pair<Token, TypePtr>> parameters;
    std::vector<StmtPtr> body;
};

#endif //CCLOX_FUN_STATEMENT_H