//
// Created by Yue Xue  on 9/5/25.
//

#ifndef CCLOX_AST_H
#define CCLOX_AST_H

#include <error.h>
#include <memory>
#include "typechecker/statement.h"
#include "chunk.h"
#include "common.h"
#include "scanner.h"
#include "scope.h"
#include "statements/print_statement.h"

class AstCompiler {
public:
    std::shared_ptr<LoxFunction> compile(std::string &&source) {
        StatementParser parser(std::move(source));
        statements = parser.parse_all();
    }

    void produce_statement(StmtPtr stmt) {

    }

    void produce_print(std::unique_ptr<PrintStatement> print_statement) {

    }

    void produce_expression();

private:
    std::vector<StmtPtr> statements;
};

#endif //CCLOX_AST_H