//
// Created by Yue Xue  on 9/3/25.
//

#ifndef CCLOX_STATEMENT_H
#define CCLOX_STATEMENT_H

#include "tokenholder.h"
#include "typechecker/expression.h"
#include "typechecker/typeparser.h"

class Statement {
public:
    virtual ~Statement() {}

    virtual TypePtr resolve_return_type() {
        return std::make_shared<PrimitiveType>(LoxTypeEnum::Unspecified);
    }
};

using StmtPtr = std::unique_ptr<Statement>;

class VarStatement: public Statement {

};

class PrintStatement: public Statement {

};

class StatementParser {
public:
    explicit StatementParser(std::shared_ptr<TokenHolder> &token_holder): tokens(token_holder) {
        type_parser = std::make_unique<TypeParser>(token_holder);
        expr_parser = std::make_unique<ExpressionParser>(token_holder);
    }

private:
    std::unique_ptr<TypeParser> type_parser;
    std::unique_ptr<ExpressionParser> expr_parser;
    std::shared_ptr<TokenHolder> tokens;
};


#endif //CCLOX_STATEMENT_H