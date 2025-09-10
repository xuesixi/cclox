//
// Created by Yue Xue  on 9/8/25.
//

#ifndef CCLOX_RETURN_STATEMENT_H
#define CCLOX_RETURN_STATEMENT_H

#include "typechecker/statement.h"

class ReturnStatement: public Statement {
public:
    friend class AstCompiler;
    explicit ReturnStatement(ExprPtr &&value, int line): value(std::move(value)), line(line) {

    }
    TypePtr resolve_return_type() override;

    void accept(AstCompiler &compiler) override;
private:
    ExprPtr value;
    TypePtr cached_return_type; // 缓存用
    int line; // 如果返回值为空的话，需要在这里读取行号
};

#endif //CCLOX_RETURN_STATEMENT_H